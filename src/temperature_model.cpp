#include "temperature_model.hpp"

#include <Eigen/Dense>

#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "hades/custom_select.hpp"
#include "hades/join.hpp"

namespace
{
    const int feature_count = 3;
}

aether::temperature_model::temperature_model(hades::connection& conn)
{
    sensor default_sensor(hades::get_one<sensor>(conn));
    sensor_at_batch sab(
        hades::get_one<sensor_at_batch>(
            conn,
            hades::where(
                "sensor_id = ? ",
                hades::row<styx::int_type>(default_sensor.get_int<attr::sensor_id>())
            )
        )
    );
    batch::id_type batch_id{sab.copy_int<attr::batch_id>()};

    styx::list points(
        hades::custom_select<
            forecast,
            attr::forecast_dt,
            attr::forecast_main_temp,
            attr::forecast_clouds_all,
            attr::log_time>(
            conn,
            "SELECT forecast_dt, "
            " forecast_main_temp, forecast_clouds_all, "
            " measured_temperature, "
            " MIN(log_time_diff) "
            " FROM ( "
            "  SELECT aether_forecast.forecast_dt AS forecast_dt, "
            "   aether_forecast_main.forecast_main_temp AS forecast_main_temp, "
            "   aether_forecast_clouds.forecast_clouds_all AS forecast_clouds_all, "
            "   aether_temperature_log.temperature AS measured_temperature, "
            "   ABS( "
            "    aether_temperature_log.log_time - "
            "    aether_forecast.forecast_dt "
            "   ) AS log_time_diff"
            "  FROM aether_forecast "
            "  JOIN aether_forecast_main "
            "  ON aether_forecast.forecast_dt = aether_forecast_main.forecast_dt "
            "  JOIN aether_forecast_clouds "
            "  ON aether_forecast.forecast_dt = aether_forecast_clouds.forecast_dt "
            "  JOIN "
            "  aether_temperature_log "
            "  ON "
            "   aether_temperature_log.log_time < "
            "    aether_forecast.forecast_dt + 3600 AND "
            "   aether_temperature_log.log_time > "
            "    aether_forecast.forecast_dt - 3600 "
            " ) "
            " GROUP BY forecast_dt "
        )
    );

    if(points.size() == 0)
        throw std::runtime_error("cannot train temperature model with no data");

    Eigen::Matrix<feature_type, Eigen::Dynamic, Eigen::Dynamic> feature_matrix(points.size(), feature_count);
    Eigen::Matrix<feature_type, Eigen::Dynamic, 1> output_vector(points.size(), 1);

    for(int i = 0; i < points.size(); ++i)
    {
        styx::object point(points.at(i));
        // Construct a feature vector.
        feature_vector_type features(feature_vector(point));
        // Append the feature vector to the training data.
        feature_matrix.row(i) = features.transpose();

        // Append the actual reading to the output vector.
        output_vector(i, 0) =
            point.get_double(temperature_log::date_attribute);
    }

    Eigen::Matrix<feature_type, Eigen::Dynamic, Eigen::Dynamic> temp =
        feature_matrix.transpose() * feature_matrix;
    m_theta =
        temp.colPivHouseholderQr().solve(
            feature_matrix.transpose() * output_vector
        );
}

aether::temperature_model::feature_type
aether::temperature_model::estimate(styx::object f)
{
    feature_vector_type features(feature_vector(f));
    return m_theta.dot(features);
}

aether::temperature_model::feature_vector_type
aether::temperature_model::feature_vector(styx::object f)
{
    feature_vector_type out(feature_count, 1);
    out <<
        // Bias unit.
        1,
        // Forecast temperature.
        f.get_double("forecast_main_temp"),
        // Forecast cloud cover.
        f.get_double("forecast_clouds_all");
    return out;
}

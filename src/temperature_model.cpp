#include "temperature_model.hpp"

#include <Eigen/Dense>

#include "atlas/log/log.hpp"
#include "hades/custom_select.hpp"
#include "hades/join.hpp"

namespace
{
    const int feature_count = 11;
}

aether::temperature_model::temperature_model(
    hades::connection& conn,
    phase::id_type phase_id
    )
{
    m_theta = theta(conn, phase_id);
    atlas::log::information("aether::temperature_model::temperature_model") <<
        "theta\n" << m_theta;
}

aether::temperature_model::feature_vector_type aether::temperature_model::theta(
    hades::connection& conn,
    phase::id_type phase_id
)
{
    styx::list points(
        hades::custom_select<
            forecast,
            hades::row<styx::int_type>,
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
            "  SELECT "
            "   aether_forecast.forecast_dt AS forecast_dt, "
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
            "    aether_forecast.forecast_dt + 5400 AND "
            "   aether_temperature_log.log_time > "
            "    aether_forecast.forecast_dt - 5400 "
            "  WHERE aether_temperature_log.phase_id = ? "
            " ) "
            " GROUP BY forecast_dt ",
            hades::row<styx::int_type>(phase_id.get_int<attr::phase_id>())
        )
    );

    if(points.size() == 0)
        throw temperature_model_exception();

    atlas::log::information("aether::temperature_model::theta") <<
        "training model with " << points.size() << " examples";

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

    atlas::log::information("aether::temperature_model::theta") <<
        "feature matrix\n" << feature_matrix;
    atlas::log::information("aether::temperature_model::theta") <<
        "output vector\n" << output_vector;

    Eigen::Matrix<feature_type, Eigen::Dynamic, Eigen::Dynamic> temp =
        feature_matrix.transpose() * feature_matrix;
    return temp.colPivHouseholderQr().solve(
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
aether::temperature_model::feature_vector(styx::object example)
{
    feature_vector_type out(feature_count, 1);
    boost::posix_time::ptime example_time =
        boost::posix_time::from_time_t(example.get_int("forecast_dt"));
    boost::posix_time::time_duration example_tod = example_time.time_of_day();
    double temperature = example.get_double("forecast_main_temp");
    out <<
        // Bias unit.
        1,
        // Forecast temperature.
        temperature,
        // Forecast cloud cover.
        example.get_double("forecast_clouds_all"),
        // Time is between midnight and 3am.
        (example_tod.hours() >= 0 && example_tod.hours() < 3) * temperature,
        // Time is between 3am and 6am.
        (example_tod.hours() >= 3 && example_tod.hours() < 6) * temperature,
        // Time is between 6am and 9am.
        (example_tod.hours() >= 6 && example_tod.hours() < 9) * temperature,
        // Time is between 9am and 12pm.
        (example_tod.hours() >= 9 && example_tod.hours() < 12) * temperature,
        // Time is between 12pm and 3pm.
        (example_tod.hours() >= 12 && example_tod.hours() < 15) * temperature,
        // Time is between 3pm and 6pm.
        (example_tod.hours() >= 15 && example_tod.hours() < 18) * temperature,
        // Time is between 6pm and 9pm.
        (example_tod.hours() >= 18 && example_tod.hours() < 21) * temperature,
        // Time is between 9pm and midnight.
        (example_tod.hours() >= 21 && example_tod.hours() < 24) * temperature;
    return out;
}

const char *aether::temperature_model_exception::what() const noexcept
{
    return "cannot train temperature model with no data";
}

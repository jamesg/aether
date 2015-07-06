#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "catch.hpp"
#include "hades/crud.ipp"
#include "hades/join.hpp"
#include "temperature_model.hpp"

SCENARIO("aether::temperature_model") {
    const int base_time = 1436086800;
    GIVEN("a database") {
        hades::connection conn(hades::connection::in_memory_database());

        aether::db::create(conn);

        // Create a batch.
        aether::batch batch;
        batch.save(conn);

        // Move the default sensor to the batch.
        aether::sensor default_sensor(hades::get_one<aether::sensor>(conn));
        aether::sensor_at_batch sab;
        sab.set_id(batch.id());
        sab.get_int<aether::attr::sensor_id>() =
            default_sensor.get_int<aether::attr::sensor_id>();
        sab.save(conn);

        GIVEN("a contrived forecast and log") {
            {
                aether::forecast f;
                aether::forecast_main fm;
                aether::forecast_clouds fc;
                aether::temperature_log tl;
                f.get_int<aether::attr::forecast_dt>() =
                    fm.get_int<aether::attr::forecast_dt>() =
                    fc.get_int<aether::attr::forecast_dt>() =
                    tl.get_double<aether::attr::log_time>() =
                    base_time;
                fm.get_double<aether::attr::forecast_main_temp>() = 20.0;
                fc.get_int<aether::attr::forecast_clouds_all>() = 50;
                tl.get_int<aether::attr::batch_id>() =
                    batch.get_int<aether::attr::batch_id>();
                tl.get_double<aether::attr::temperature>() = 21.0;
                f.save(conn);
                fm.save(conn);
                fc.save(conn);
                tl.save(conn);
            }

            {
                aether::forecast f;
                aether::forecast_main fm;
                aether::forecast_clouds fc;
                aether::temperature_log tl;
                f.get_int<aether::attr::forecast_dt>() =
                    fm.get_int<aether::attr::forecast_dt>() =
                    fc.get_int<aether::attr::forecast_dt>() =
                    tl.get_double<aether::attr::log_time>() =
                    base_time + 10800;
                fm.get_double<aether::attr::forecast_main_temp>() = 22.0;
                fc.get_int<aether::attr::forecast_clouds_all>() = 50;
                tl.get_int<aether::attr::batch_id>() =
                    batch.get_int<aether::attr::batch_id>();
                tl.get_double<aether::attr::temperature>() = 23.0;
                f.save(conn);
                fm.save(conn);
                fc.save(conn);
                tl.save(conn);
            }

            aether::temperature_model model(conn);

            // Test on seen data.
            styx::list seen(
                hades::equi_outer_join<
                    aether::forecast,
                    aether::forecast_main,
                    aether::forecast_clouds>(
                    conn,
                    hades::order_by("aether_forecast.forecast_dt ASC")
                )
            );
            REQUIRE(seen.size() == 2);
            float estimate_cmp;
            estimate_cmp = model.estimate(styx::object(seen.at(0))) / 21;
            REQUIRE(estimate_cmp > 0.99);
            REQUIRE(estimate_cmp < 1.01);
            estimate_cmp = model.estimate(styx::object(seen.at(1))) / 23;
            REQUIRE(estimate_cmp > 0.99);
            REQUIRE(estimate_cmp < 1.01);

            // Test on unseen data.
            {
                aether::forecast_main fm;
                fm.get_double<aether::attr::forecast_main_temp>() = 21.0;
                fm.get_int<aether::attr::forecast_clouds_all>() = 50;
                // A temperature between the two in the training set should
                // result in an estimate between the two in the temperature log.
                REQUIRE(model.estimate(fm) > 21);
                REQUIRE(model.estimate(fm) < 23);
            }
        }
    }
}

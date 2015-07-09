#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "catch.hpp"
#include "hades/crud.ipp"
#include "hades/join.hpp"
#include "temperature_model.hpp"

namespace
{
    class database_fixture
    {
    public:
        database_fixture() :
            m_connection(hades::connection::in_memory_database())
        {
            aether::db::create(conn());

            // Create a phase.
            m_phase.save(conn());

            // Create a batch.
            m_batch.save(conn());

            // Move the batch to the phase.
            aether::batch_phase bp;
            bp.get_int<aether::attr::batch_id>() =
                m_batch.get_int<aether::attr::batch_id>();
            bp.get_int<aether::attr::phase_id>() =
                m_phase.get_int<aether::attr::phase_id>();
            bp.save(conn());

            // Move the default sensor to the batch.
            aether::sensor default_sensor(hades::get_one<aether::sensor>(conn()));
            aether::sensor_at_batch sab;
            sab.set_id(m_batch.id());
            sab.get_int<aether::attr::sensor_id>() =
                default_sensor.get_int<aether::attr::sensor_id>();
            sab.save(conn());
        }
        hades::connection& conn()
        {
            return m_connection;
        }
        aether::batch batch()
        {
            return m_batch;
        }
        aether::phase phase()
        {
            return m_phase;
        }
    private:
        hades::connection m_connection;
        aether::batch m_batch;
        aether::phase m_phase;
    };

    const int base_time = 1436086800;
}

SCENARIO("aether::temperature_model") {
    // A forecast in which only the temperature changes.
    GIVEN("a contrived forecast and log") {
        database_fixture fixture;
        hades::connection& conn = fixture.conn();
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
            tl.get_int<aether::attr::phase_id>() =
                fixture.phase().get_int<aether::attr::phase_id>();
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
            tl.get_int<aether::attr::phase_id>() =
                fixture.phase().get_int<aether::attr::phase_id>();
            tl.get_double<aether::attr::temperature>() = 23.0;
            f.save(conn);
            fm.save(conn);
            fc.save(conn);
            tl.save(conn);
        }

        aether::temperature_model model(conn, fixture.phase().id());

        // Test on seen data.
        // There are only two points, so the model should fit perfectly.
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

    // A forecast in which both the temperature an cloud cover change.  The
    // measured temperature will be greater when there is less cloud cover
    // (inspired by a sensor placed in a greenhouse).
    GIVEN("a more complex forecast") {
        database_fixture fixture;
        hades::connection& conn = fixture.conn();
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
            fc.get_int<aether::attr::forecast_clouds_all>() = 100;
            tl.get_int<aether::attr::phase_id>() =
                fixture.phase().get_int<aether::attr::phase_id>();
            tl.get_double<aether::attr::temperature>() = 20.0;
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
            fm.get_double<aether::attr::forecast_main_temp>() = 21.0;
            fc.get_int<aether::attr::forecast_clouds_all>() = 0;
            tl.get_int<aether::attr::phase_id>() =
                fixture.phase().get_int<aether::attr::phase_id>();
            tl.get_double<aether::attr::temperature>() = 27.0;
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
                base_time + 21600;
            fm.get_double<aether::attr::forecast_main_temp>() = 22.0;
            fc.get_int<aether::attr::forecast_clouds_all>() = 50;
            tl.get_int<aether::attr::phase_id>() =
                fixture.phase().get_int<aether::attr::phase_id>();
            tl.get_double<aether::attr::temperature>() = 25.0;
            f.save(conn);
            fm.save(conn);
            fc.save(conn);
            tl.save(conn);
        }

        aether::temperature_model model(conn, fixture.phase().id());

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
        REQUIRE(seen.size() == 3);
        // 0: 20, 1: 27, 2: 25
        REQUIRE(model.estimate(styx::object(seen.at(0))) > 19);
        REQUIRE(model.estimate(styx::object(seen.at(0))) < 21);
        REQUIRE(model.estimate(styx::object(seen.at(1))) > 26);
        REQUIRE(model.estimate(styx::object(seen.at(1))) < 28);
        REQUIRE(model.estimate(styx::object(seen.at(2))) > 24);
        REQUIRE(model.estimate(styx::object(seen.at(2))) < 26);

        // Test on unseen data.
        {
            aether::forecast_main fm;
            fm.get_double<aether::attr::forecast_main_temp>() = 20.0;
            fm.get_int<aether::attr::forecast_clouds_all>() = 75;
            // The measured temperature should be greater with less cloud cover.
            REQUIRE(model.estimate(fm) > 20);
            REQUIRE(model.estimate(fm) < 23);
        }
        {
            aether::forecast_main fm;
            fm.get_double<aether::attr::forecast_main_temp>() = 21.0;
            fm.get_int<aether::attr::forecast_clouds_all>() = 25;
            // The measured temperature should be cooler than 27 because one
            // training sample had forecast=21, measured=27 with no cloud cover.
            REQUIRE(model.estimate(fm) > 21);
            REQUIRE(model.estimate(fm) < 27);
        }
    }
}

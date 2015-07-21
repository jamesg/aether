#include "sensor_api.hpp"

#include "aether/db.hpp"
#include "hades/custom_select_one.hpp"
#include "hades/exists.hpp"
#include "hades/get_by_id.hpp"
#include "hades/get_one.hpp"

aether::sensor_api::sensor_api(hades::connection& conn) :
    atlas::api::server(nullptr)
{
    install_sensor_api(conn);
}

aether::sensor_api::sensor_api(
        boost::shared_ptr<boost::asio::io_service> io,
        hades::connection& conn
        ) :
    atlas::api::server(io)
{
    install_sensor_api(conn);
}

void aether::sensor_api::install_sensor_api(hades::connection& conn)
{
    // Record the time sensor_id last sent data.
    auto log_receipt = [&conn](const styx::int_type sensor_id) {
        sensor_data_received(sensor::id_type{sensor_id}).save(conn);
    };

    install<std::string>(
        "status",
        []() {
            return "ok";
        }
    );
    // Is the server ready to receive data from this sensor?
    install<bool, styx::int_type>(
        "ready_to_receive",
        [&conn, log_receipt](const styx::int_type sensor_id) {
            log_receipt(sensor_id);
            return hades::exists<sensor_at_batch>(conn);
        }
    );
    install<bool, styx::int_type, styx::int_type>(
        "record_moisture",
        [&conn, log_receipt](const styx::int_type sensor_id, const styx::int_type moisture) {
            log_receipt(sensor_id);
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
            batch::id_type id{sab.copy_int<attr::batch_id>()};

            // Do not record a moisture reading if a moisture reading was
            // recorded less than 30 minutes ago.
            try
            {
                if(
                    atlas::db::most_recent<moisture_log>(conn, id).date() >
                    atlas::db::date::to_unix_time(
                        boost::posix_time::second_clock::universal_time() -
                        boost::posix_time::minutes(30)
                    )
                  )
                    return false;
            }
            catch(const std::exception&)
            {
            }
            moisture_log log(id);
            log.record(moisture, conn);
            return true;
        }
        );
    install<bool, styx::int_type, double>(
        "record_temperature",
        [&conn, log_receipt](const styx::int_type sensor_id, const double temperature) {
            log_receipt(sensor_id);
            sensor default_sensor(hades::get_one<sensor>(conn));
            sensor_at_phase sap(
                hades::get_one<sensor_at_phase>(
                    conn,
                    hades::where(
                        "sensor_id = ? ",
                        hades::row<styx::int_type>(default_sensor.get_int<attr::sensor_id>())
                    )
                )
            );
            phase::id_type id{sap.copy_int<attr::phase_id>()};

            // Do not record a temperature if a temperature was recorded less
            // than 30 minutes ago.
            try
            {
                if(
                    atlas::db::most_recent<temperature_log>(conn, id).date() >
                    atlas::db::date::to_unix_time(
                        boost::posix_time::second_clock::universal_time() -
                        boost::posix_time::minutes(30)
                    )
                  )
                    return false;
            }
            catch(const std::exception&)
            {
            }

            temperature_log log(id);
            log.record(temperature, conn);
            return true;
        }
    );
    install<std::string, styx::int_type>(
        "variety_cname",
        [&conn, log_receipt](const styx::int_type sensor_id) {
            log_receipt(sensor_id);
            sensor variety_sensor(
                hades::get_by_id<sensor>(conn, sensor::id_type{sensor_id})
            );
            sensor_at_batch sab(
                hades::get_one<sensor_at_batch>(
                    conn,
                    hades::where(
                        "sensor_id = ? ",
                        hades::row<styx::int_type>(variety_sensor.get_int<attr::sensor_id>())
                    )
                )
            );
            batch b(
                hades::get_by_id<batch>(
                    conn,
                    batch::id_type{sab.get_int<attr::batch_id>()}
                )
            );
            kb::variety v(
                hades::get_by_id<kb::variety>(
                    conn,
                    kb::variety::id_type{b.get_int<attr::kb_variety_id>()}
                    )
                );
            return v.get_string<attr::kb_variety_cname>();
        }
    );
    install<std::string>(
        "location",
        [&conn]() {
            location l = hades::get_one<location>(conn);
            return l.get_string<attr::location_city>();
        }
    );
    install<std::string, styx::int_type>(
        "phase",
        [&conn, log_receipt](const styx::int_type sensor_id) {
            log_receipt(sensor_id);
            sensor variety_sensor(
                hades::get_by_id<sensor>(conn, sensor::id_type{sensor_id})
            );
            phase p = hades::custom_select_one<
                phase,
                hades::row<styx::int_type>,
                attr::phase_desc>
            (
                conn,
                "SELECT aether_phase.phase_desc FROM aether_sensor_at_batch "
                " LEFT OUTER JOIN aether_batch_phase "
                " ON aether_sensor_at_batch.batch_id = aether_batch_phase.batch_id "
                " LEFT OUTER JOIN aether_phase "
                " ON aether_batch_phase.phase_id = aether_phase.phase_id "
                " WHERE aether_sensor_at_batch.sensor_id = ? ",
                hades::row<styx::int_type>(variety_sensor.get_int<attr::sensor_id>())
            );
            return p.get_string<attr::phase_desc>();
        }
    );
}

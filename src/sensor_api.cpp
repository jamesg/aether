#include "sensor_api.hpp"

#include "aether/db.hpp"
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
    install<std::string>(
        "status",
        []() {
            return "ok";
        }
    );
    // Is the server ready to receive data from this sensor?
    install<bool>(
        "ready_to_receive",
        [&conn]() {
            return hades::exists<sensor_at_batch>(conn);
        }
    );
    install<bool, int>(
        "record_moisture",
        [&conn](const int moisture) {
            sensor default_sensor(hades::get_one<sensor>(conn));
            sensor_at_batch sab(
                hades::get_one<sensor_at_batch>(
                    conn,
                    hades::where(
                        "sensor_id = ? ",
                        hades::row<int>(default_sensor.get_int<attr::sensor_id>())
                    )
                )
            );
            moisture_log log(
                batch::id_type{sab.copy_int<attr::batch_id>()}
            );
            log.record(moisture, conn);
            return true;
        }
        );
    install<bool, double>(
        "record_temperature",
        [&conn](const double temperature) {
            sensor default_sensor(hades::get_one<sensor>(conn));
            sensor_at_batch sab(
                hades::get_one<sensor_at_batch>(
                    conn,
                    hades::where(
                        "sensor_id = ? ",
                        hades::row<int>(default_sensor.get_int<attr::sensor_id>())
                    )
                )
            );
            temperature_log log(
                batch::id_type{sab.copy_int<attr::batch_id>()}
            );
            log.record(temperature, conn);
            return true;
        }
    );
    install<std::string>(
        "variety_cname",
        [&conn]() {
            sensor default_sensor(hades::get_one<sensor>(conn));
            sensor_at_batch sab(
                hades::get_one<sensor_at_batch>(
                    conn,
                    hades::where(
                        "sensor_id = ? ",
                        hades::row<int>(default_sensor.get_int<attr::sensor_id>())
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
}

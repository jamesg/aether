#include "sensor_api.hpp"

#include "aether/db.hpp"
#include "hades/get_by_id.hpp"

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
    install<bool, int>(
            "record_moisture",
            [&conn](const int moisture) {
                sensor default_sensor(hades::get_one<sensor>(conn));
                sensor_at_batch sab(
                    hades::get_by_id<sensor_at_batch>(conn, default_sensor.id())
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
                    hades::get_by_id<sensor_at_batch>(conn, default_sensor.id())
                );
                temperature_log log(
                    batch::id_type{sab.copy_int<attr::batch_id>()}
                );
                log.record(temperature, conn);
                return true;
            }
            );
    install<std::string, int>(
            "variety_cname",
            [&conn](const int variety_id) {
                kb::variety v(
                    hades::get_by_id<kb::variety>(
                        conn,
                        kb::variety::id_type{variety_id}
                        )
                    );
                return v.get_string<attr::kb_variety_cname>();
            }
            );
}

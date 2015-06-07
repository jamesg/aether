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
                return true;
            }
            );
    install<bool, double>(
            "record_temperature",
            [&conn](const double temperature) {
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

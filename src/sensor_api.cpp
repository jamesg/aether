#include "sensor_api.hpp"

aether::sensor_api::sensor_api(hades::connection& conn) :
    atlas::api::server(nullptr)
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
                return "TODO";
            }
            );
}


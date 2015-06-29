#include "city_to_location.hpp"

#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "hades/mkstr.hpp"
#include "styx/serialise_json.hpp"

void aether::openweathermap::city_to_location(
        boost::shared_ptr<boost::asio::io_service> io,
        const std::string& city_name,
        boost::function<void(location)> success,
        boost::function<void(std::string)> failure
        )
{
    atlas::log::information("aether::openweathermap::city_to_location") << "requesting " <<
        city_name;

    atlas::http::get_json(
        io,
        hades::mkstr() <<
            "http://api.openweathermap.org/data/2.5/forecast?q=" <<
            city_name,
        [success, failure, city_name](styx::element e) {
            try
            {
                styx::object f(e);

                const int cod = f.copy_int("cod");
                if(cod < 200 || cod > 299)
                {
                    atlas::log::information("aether::openweathermap::city_to_location") << "code " <<
                        cod;
                    failure(hades::mkstr() << "error code " << cod);
                }
                else
                {
                    const double lat = f.get_object("city").get_object("coord").get_double("lat");
                    const double lon = f.get_object("city").get_object("coord").get_double("lon");
                    atlas::log::information("aether::openweathermap::city_to_location") <<
                        "found " << city_name << " at " << lon << " N " <<
                        lat << " E";

                    location l;
                    l.get_string<attr::location_city>() = f.get_object("city").get_string("name");
                    l.get_double<attr::location_lon>() = lon;
                    l.get_double<attr::location_lat>() = lat;
                    success(l);
                }
            }
            catch(const std::exception& e)
            {
                atlas::log::warning("aether::openweathermap::city_to_location") << "parsing forecast: " << e.what();
                failure(hades::mkstr() << "exception " << e.what());
            }
        },
        [failure](const std::string& str) {
            atlas::log::warning("aether::openweathermap::city_to_location") <<
                "openweathermap api request failed: " << str;
            failure(str);
        }
        );
}

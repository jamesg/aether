#include "city_to_location.hpp"

#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "hades/mkstr.hpp"
#include "styx/serialise_json.hpp"

#include "forecast.hpp"

void aether::openweathermap::city_to_location(
        boost::shared_ptr<boost::asio::io_service> io,
        const std::string& city_name,
        boost::function<void(location)> success,
        boost::function<void(std::string)> failure
        )
{
    atlas::log::information("aether::openweathermap::city_to_location") << "requesting " <<
        city_name;

    atlas::http::get(
        io,
        hades::mkstr() <<
            "http://api.openweathermap.org/data/2.5/forecast?q=" <<
            city_name,
        [success, failure](const std::string& str) {
            try
            {

                std::string::size_type open_brace = str.find('{');
                if(open_brace == std::string::npos)
                    throw std::runtime_error("opening brace not found");
                styx::element forecast_e = styx::parse_json(
                    std::string(str, open_brace, std::string::npos)
                    );

                atlas::log::information("openweathermap::city_to_location") <<
                    "forecast " << styx::serialise_json(forecast_e);
                forecast f(forecast_e);

                if(f.cod() < 200 || f.cod() > 299)
                {
                    atlas::log::information("openweathermap::city_to_location") << "code " <<
                        f.cod();
                    failure(hades::mkstr() << "error code " << f.cod());
                }
                else
                {
                    atlas::log::information("openweathermap::city_to_location") << "found " <<
                        f.city().coord().lat() << " " << f.city().coord().lon();

                    location l;
                    l.get_double<attr::location_lon>() = f.city().coord().lon();
                    l.get_double<attr::location_lat>() = f.city().coord().lat();
                    success(l);
                }
            }
            catch(const std::exception& e)
            {
                atlas::log::warning("openweathermap::city_to_location") << "parsing forecast: " << e.what();
                failure(hades::mkstr() << "exception " << e.what());
            }
        },
        [failure](const std::string& str) {
            atlas::log::warning("openweathermap::city_to_location") <<
                "openweathermap api request failed";
            failure("Error contacting external server.");
        }
        );
}


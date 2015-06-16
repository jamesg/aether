#include "retrieve_forecast.hpp"

#include "atlas/http/client.hpp"
#include "atlas/log/log.hpp"
#include "hades/mkstr.hpp"
#include "styx/styx.hpp"

#include "forecast.hpp"

void aether::openweathermap::retrieve_forecast(
        boost::shared_ptr<boost::asio::io_service> io,
        float lat,
        float lon,
        boost::function<void(openweathermap::forecast&)> success,
        boost::function<void(const std::string&)> failure
        )
{
    atlas::http::get(
        io,
        hades::mkstr() <<
            "http://api.openweathermap.org/data/2.5/forecast?lat=" <<
            lat << "&lon=" << lon,
        [success, failure](const std::string& body) {

            std::string::size_type open_brace = body.find('{');
            std::string::size_type close_brace = body.find_last_of('}');
            if(
                open_brace == std::string::npos ||
                close_brace == std::string::npos
                )
            {
                failure("error parsing json");
                return;
            }

            try
            {
                styx::element e =
                    styx::parse_json(
                        std::string(body, open_brace, close_brace-open_brace+1)
                        );
                openweathermap::forecast f(e);
                success(f);
            }
            catch(const std::exception&)
            {
                failure("error parsing json");
            }
        },
        [failure](const std::string error) {
            failure(error);
        }
        );
}


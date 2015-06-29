#include "weather_router.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "aether/db.hpp"
#include "hades/custom_select_one.hpp"
#include "hades/join.hpp"

namespace
{
    int to_unix_time(boost::posix_time::ptime t)
    {
        return (
            t -
            boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))
        ).total_seconds();
    }
    styx::list forecast_range(
        hades::connection& conn,
        boost::posix_time::ptime from,
        boost::posix_time::ptime to
    )
    {
        return hades::equi_outer_join<
            aether::forecast,
            aether::forecast_main,
            aether::forecast_clouds,
            aether::forecast_rain,
            aether::forecast_weather,
            aether::forecast_wind>(
            conn,
            hades::filter(
                hades::where(
                    "aether_forecast.forecast_dt >= ? AND "
                    "aether_forecast.forecast_dt < ? ",
                    hades::row<int, int>(
                        to_unix_time(from),
                        to_unix_time(to)
                    )
                ),
                hades::order_by("aether_forecast.forecast_dt ASC")
                )
        );
    }
}

aether::weather_router::weather_router(hades::connection& conn) {
    install<int>(
        atlas::http::matcher("/([0-9]+)", "GET"),
        [&conn](const int dt) {
            styx::list points = hades::equi_outer_join<
                forecast,
                forecast_main,
                forecast_clouds,
                forecast_rain,
                forecast_weather,
                forecast_wind>(
                    conn,
                    hades::filter(
                        hades::where(
                            "aether_forecast.forecast_dt = ?",
                            hades::row<int>(dt)
                            ),
                        hades::order_by("aether_forecast.forecast_dt ASC", 1)
                        )
                    );
            return atlas::http::json_response(points.at(0));
        }
        );
    install<int>(
        atlas::http::matcher("/([0-9]+)/nearest", "GET"),
        [&conn](const int dt) {
        // Find the nearest dt.
        forecast nearest = hades::custom_select_one<
            forecast,
            hades::row<int>,
            attr::forecast_dt>(
                conn,
                "SELECT forecast_dt FROM aether_forecast "
                " ORDER BY ABS(? - aether_forecast.forecast_dt) ASC LIMIT 1",
                hades::row<int>(dt)
                );
        // Return the forecast.
        styx::list points = hades::equi_outer_join<
            forecast,
            forecast_main,
            forecast_clouds,
            forecast_rain,
            forecast_weather,
            forecast_wind>(
                conn,
                hades::filter(
                    hades::where(
                        "aether_forecast.forecast_dt = ?",
                        hades::row<int>(nearest.get_int<attr::forecast_dt>())
                        ),
                    hades::order_by("aether_forecast.forecast_dt ASC", 1)
                    )
                );
            return atlas::http::json_response(points.at(0));
        }
        );
    install_get<>(
        atlas::http::matcher("/today", "GET"),
        [&conn](std::map<std::string, std::string> params) {
            int timezone_offset_s = 0;
            if(params.find("timezone") == params.end())
                atlas::log::warning("aether::weather_router") <<
                    "no timezone specified; using 0 (UTC)";
            else
                timezone_offset_s =
                    boost::lexical_cast<int>(params.find("timezone")->second) * 60;
            return atlas::http::json_response(
                forecast_range(
                    conn,
                    boost::posix_time::ptime(boost::gregorian::day_clock::universal_day()) +
                        boost::posix_time::seconds(timezone_offset_s),
                    boost::posix_time::ptime(boost::gregorian::day_clock::universal_day()) +
                        boost::gregorian::days(1) +
                        boost::posix_time::seconds(timezone_offset_s)
                )
            );
        }
        );
    install_get<int>(
        atlas::http::matcher("/day/([0-9]+)", "GET"),
        [&conn](std::map<std::string, std::string> params, int day) {
            int timezone_offset_s = 0;
            if(params.find("timezone") == params.end())
                atlas::log::warning("aether::weather_router") <<
                    "no timezone specified; using 0 (UTC)";
            else
                timezone_offset_s =
                    boost::lexical_cast<int>(params.find("timezone")->second) * 60;
            return atlas::http::json_response(
                forecast_range(
                    conn,
                    boost::posix_time::ptime(boost::gregorian::day_clock::universal_day()) +
                        boost::gregorian::days(day) +
                        boost::posix_time::seconds(timezone_offset_s),
                    boost::posix_time::ptime(boost::gregorian::day_clock::universal_day()) +
                        boost::gregorian::days(day + 1) +
                        boost::posix_time::seconds(timezone_offset_s)
                )
            );
        }
        );
}

#include "weather_router.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "aether/db.hpp"
#include "hades/crud.ipp"
#include "hades/custom_select_one.hpp"
#include "hades/exists.hpp"
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
                    hades::row<styx::int_type, styx::int_type>(
                        to_unix_time(from),
                        to_unix_time(to)
                    )
                ),
                hades::order_by("aether_forecast.forecast_dt ASC")
                )
        );
    }

    styx::list detailed_forecast(
        hades::connection& conn,
        const boost::gregorian::date date,
        const int timezone_offset_s
    )
    {
        return forecast_range(
            conn,
            boost::posix_time::ptime(date) +
                boost::posix_time::seconds(timezone_offset_s),
            boost::posix_time::ptime(date) +
                boost::gregorian::days(1) +
                boost::posix_time::seconds(timezone_offset_s)
        );
    }

    styx::list daily_forecast_range(
        hades::connection& conn,
        const boost::gregorian::date from,
        const int timezone_offset_s
    )
    {
        styx::list f = aether::daily_forecast::get_collection(
            conn,
            hades::filter(
                hades::where(
                    "aether_daily_forecast.forecast_dt >= ? ",
                    hades::row<styx::int_type>(
                        to_unix_time(
                            boost::posix_time::ptime(from) +
                            boost::posix_time::seconds(timezone_offset_s)
                        )
                    )
                ),
                hades::order_by("aether_daily_forecast.forecast_dt")
            )
        );
        for(int i = 0; i < f.size(); ++i)
        {
            styx::object& point = f.get<styx::object>(i);
            // Make life a lot easier on the client side by adding the forecast
            // date as an ISO formatted string.
            point.get_string("date") = boost::gregorian::to_iso_extended_string(
                (boost::posix_time::from_time_t(point.copy_int("forecast_dt")) +
                    boost::posix_time::seconds(timezone_offset_s)).date()
            );
            // Tell the client if there is a detailed forecast available for
            // this day.
            point.get_bool("detailed_available") = hades::exists<aether::forecast>(
                conn,
                hades::where(
                    "forecast_dt >= ? AND forecast_dt < ?",
                    hades::row<styx::int_type, styx::int_type>(
                        to_unix_time(
                            boost::posix_time::from_time_t(point.copy_int("forecast_dt")) +
                            boost::posix_time::seconds(timezone_offset_s)
                        ),
                        to_unix_time(
                            boost::posix_time::from_time_t(point.copy_int("forecast_dt")) +
                            boost::gregorian::days(1) +
                            boost::posix_time::seconds(timezone_offset_s)
                        )
                    )
                )
            );
        }
        return f;
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
                            hades::row<styx::int_type>(dt)
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
            hades::row<styx::int_type>,
            attr::forecast_dt>(
                conn,
                "SELECT forecast_dt FROM aether_forecast "
                " ORDER BY ABS(? - aether_forecast.forecast_dt) ASC LIMIT 1",
                hades::row<styx::int_type>(dt)
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
                        hades::row<styx::int_type>(nearest.get_int<attr::forecast_dt>())
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
    // Get a summary weather forecast for all available days.
    install_get<>(
        atlas::http::matcher("/day", "GET"),
        [&conn](std::map<std::string, std::string> params) {
            int timezone_offset_s = 0;
            if(params.find("timezone") == params.end())
                atlas::log::warning("aether::weather_router") <<
                    "no timezone specified; using 0 (UTC)";
            else
                timezone_offset_s =
                    boost::lexical_cast<int>(params.find("timezone")->second) * 60;
            return atlas::http::json_response(
                daily_forecast_range(
                    conn,
                    boost::gregorian::day_clock::universal_day(),
                    timezone_offset_s
                )
            );
        }
    );
    // Get a detailed weather forecast for a single day.
    install_get<std::string>(
        atlas::http::matcher("/day/([^/]+)", "GET"),
        [&conn](std::map<std::string, std::string> params, std::string date) {
            int timezone_offset_s = 0;
            if(params.find("timezone") == params.end())
                atlas::log::warning("aether::weather_router") <<
                    "no timezone specified; using 0 (UTC)";
            else
                timezone_offset_s =
                    boost::lexical_cast<int>(params.find("timezone")->second) * 60;
            return atlas::http::json_response(
                detailed_forecast(
                    conn,
                    boost::gregorian::from_string(date),
                    timezone_offset_s
                )
            );
        }
        );
}

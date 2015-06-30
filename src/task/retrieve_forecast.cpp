#include "retrieve_forecast.hpp"

#include <boost/bind.hpp>

#include "aether/db.hpp"
#include "atlas/http/client.hpp"
#include "atlas/log/log.hpp"
#include "hades/crud.ipp"
#include "hades/get_one.hpp"
#include "styx/cast.hpp"

boost::shared_ptr<aether::task::retrieve_forecast>
aether::task::retrieve_forecast::create(
    atlas::task::io_service_type io_,
    atlas::task::callback_function_type callback,
    hades::connection& conn
    )
{
    return boost::shared_ptr<retrieve_forecast>(
            new retrieve_forecast(io_, callback, conn)
            );
}

aether::task::retrieve_forecast::retrieve_forecast(
    atlas::task::io_service_type io_,
    atlas::task::callback_function_type callback,
    hades::connection& conn
    ) :
    atlas::task::base(io_, callback),
    atlas::task::has_connection(conn)
{
}

void aether::task::retrieve_forecast::run()
{
    request_daily_forecast();
}

void aether::task::retrieve_forecast::request_daily_forecast()
{
    atlas::log::information("aether::task::retrieve_forecast::request_daily_forecast") <<
        "request";
    try
    {
        location l(hades::get_one<location>(connection()));

        atlas::http::get_json(
            io_ptr(),
            hades::mkstr() <<
                "http://api.openweathermap.org/data/2.5/forecast/daily?lat=" <<
                l.get_double<attr::location_lat>() << "&lon=" <<
                l.get_double<attr::location_lon>(),
            boost::bind(
                &retrieve_forecast::daily_forecast_received,
                shared_from_this(),
                _1
            ),
            boost::bind(
                &retrieve_forecast::daily_forecast_error,
                shared_from_this(),
                _1
            )
        );
    }
    catch(const hades::exception&)
    {
        atlas::log::warning("aether::task::retrieve_forecast::request_daily_forecast") <<
            "could not retrieve forecast (location not set)";
        mark_finished();
    }
    catch(const std::exception&)
    {
        atlas::log::warning("aether::task::retrieve_forecast::request_daily_forecast") <<
            "could not retrieve forecast (unknown error)";
        mark_finished();
    }
}

void aether::task::retrieve_forecast::daily_forecast_received(
        styx::element e
        )
{
    try
    {
        styx::object f(e);
        for(styx::object point : f.get_list("list"))
        {
            daily_forecast f_db;
            f_db.get_int<attr::forecast_dt>() = point.copy_int("dt");

            {
                styx::list weather(point.get_list("weather"));
                if(weather.size() > 0)
                {
                    styx::object first = weather.at(0);
                    f_db.get_string<attr::forecast_weather_main>() =
                        first.get_string("main");
                    f_db.get_string<attr::forecast_weather_description>() =
                        first.get_string("description");
                }
            }

            {
                styx::object& temp(point.get_object("temp"));
                f_db.get_double<attr::forecast_temp_day>() =
                    temp.get_double("day");
                f_db.get_double<attr::forecast_temp_night>() =
                    temp.get_double("night");
                f_db.get_double<attr::forecast_wind_speed>() =
                    point.get_double("speed");
                f_db.get_int<attr::forecast_wind_deg>() =
                    point.get_int("deg");
                f_db.get_int<attr::forecast_clouds_all>() =
                    point.get_int("clouds");
            }

            f_db.save(connection());
        }
        atlas::log::information("aether::task::retrieve_forecast::daily_forecast_received") <<
            "saved daily forecast";
    }
    catch(const std::exception& e)
    {
        atlas::log::error("aether::task::retrieve_forecast::daily_forecast_received") <<
            e.what();
    }

    request_forecast();
}

void aether::task::retrieve_forecast::daily_forecast_error(const std::string& message)
{
    atlas::log::warning("aether::task::retrieve_forecast") <<
        "could not retrieve daily forecast (communication error): " << message;

    request_forecast();
}

void aether::task::retrieve_forecast::request_forecast()
{
    atlas::log::information("aether::task::retrieve_forecast::request_forecast") <<
        "request";
    try
    {
        location l(hades::get_one<location>(connection()));
        atlas::http::get_json(
            io_ptr(),
            hades::mkstr() <<
                "http://api.openweathermap.org/data/2.5/forecast?lat=" <<
                l.get_double<attr::location_lat>() << "&lon=" <<
                l.get_double<attr::location_lon>(),
            boost::bind(
                &retrieve_forecast::forecast_received,
                shared_from_this(),
                _1
            ),
            boost::bind(
                &retrieve_forecast::forecast_error,
                shared_from_this(),
                _1
            )
        );
    }
    catch(const hades::exception&)
    {
        atlas::log::warning("aether::task::retrieve_forecast::request_forecast") <<
            "could not retrieve forecast (location not set)";
        mark_finished();
    }
    catch(const std::exception&)
    {
        atlas::log::warning("aether::task::retrieve_forecast::request_forecast") <<
            "could not retrieve forecast (unknown error)";
        mark_finished();
    }
}

void aether::task::retrieve_forecast::forecast_received(
        styx::element e
        )
{
    styx::object f(e);
    for(styx::object point : f.get_list("list"))
    {
        forecast f_db;
        f_db.get_int<attr::forecast_dt>() = point.copy_int("dt");
        f_db.save(connection());

        if(point.has_key("main"))
        {
            const styx::object& main = point.get_object("main");
            forecast_main f_main;
            f_main.get_int<attr::forecast_dt>() = point.copy_int("dt");
            f_main.get_double<attr::forecast_main_temp>() = main.copy_double("temp");
            f_main.get_double<attr::forecast_main_temp_min>() = main.copy_double("temp_min");
            f_main.get_double<attr::forecast_main_temp_max>() = main.copy_double("temp_max");
            f_main.get_int<attr::forecast_main_humidity>() = main.copy_int("humidity");
            f_main.get_double<attr::forecast_main_pressure>() = main.copy_double("pressure");
            f_main.get_double<attr::forecast_main_sea_level>() = main.copy_double("sea_level");
            f_main.get_double<attr::forecast_main_grnd_level>() = main.copy_double("grnd_level");
            f_main.save(connection());
        }
        if(point.has_key("clouds"))
        {
            const styx::object& clouds = point.get_object("clouds");
            forecast_clouds f_clouds;
            f_clouds.get_int<attr::forecast_dt>() = point.copy_int("dt");
            f_clouds.get_int<attr::forecast_clouds_all>() = clouds.copy_int("all");
            f_clouds.save(connection());
        }
        if(point.has_key("rain"))
        {
            const styx::object& rain = point.get_object("rain");
            forecast_rain f_rain;
            f_rain.get_int<attr::forecast_dt>() = point.copy_int("dt");
            f_rain.get_int<attr::forecast_rain>() = rain.copy_int("3h");
            f_rain.save(connection());
        }
        if(point.has_key("weather"))
        {
            const styx::object weather = styx::cast<styx::object>(point.get_list("weather").at(0));
            forecast_weather f_weather;
            f_weather.get_int<attr::forecast_dt>() = point.copy_int("dt");
            f_weather.get_string<attr::forecast_weather_main>() = weather.copy_string("main");
            f_weather.get_string<attr::forecast_weather_description>() = weather.copy_string("description");
            f_weather.save(connection());
        }
        if(point.has_key("wind"))
        {
            const styx::object& wind = point.get_object("wind");
            forecast_wind f_wind;
            f_wind.get_int<attr::forecast_dt>() = point.copy_int("dt");
            f_wind.get_double<attr::forecast_wind_speed>() = wind.copy_double("speed");
            f_wind.get_double<attr::forecast_wind_deg>() = wind.copy_double("deg");
            f_wind.get_double<attr::forecast_wind_gust>() = wind.copy_double("gust");
            f_wind.save(connection());
        }
    }
    atlas::log::information("aether::task::retrieve_forecast") <<
        "saved forecast";
    mark_finished();
}

void aether::task::retrieve_forecast::forecast_error(const std::string& message)
{
    atlas::log::warning("aether::task::retrieve_forecast") <<
        "could not retrieve forecast (communication error): " << message;
    mark_finished();
}

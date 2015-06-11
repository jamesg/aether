#include <boost/date_time/posix_time/posix_time.hpp>
#include "catch.hpp"

#include "atlas/log/log.hpp"
#include "styx/element.hpp"
#include "styx/styx.hpp"

#include "openweathermap/forecast.hpp"
#include "openweathermap/point.hpp"
#include "openweathermap/request_forecast.hpp"

/*
 * Test the retrieval of weather forecast data from the Open Weather Map
 * service.  Both the availability and format of the data are tested.  As the
 * test case operates on live data, values are only checked within a range of
 * realistic weather conditions.
 */
SCENARIO("openweathermap::forecast") {
    WHEN("a forecast request is made") {
        aether::openweathermap::forecast forecast;

        boost::shared_ptr<boost::asio::io_service> io(
                new boost::asio::io_service
                );
        aether::openweathermap::request_forecast(
                io,
                51,
                0,
                [&forecast](aether::openweathermap::forecast& forecast_)
                {
                    forecast = forecast_;
                    atlas::log::information("aether::openweathermap::forecast test") <<
                        "retrieved forecast";
                },
                [](const std::string& error) {
                    atlas::log::information("aether::openweathermap::forecast test") <<
                        "error " << error;
                }
                );
        io->run();

        THEN("the result contains sensible data") {
            REQUIRE(forecast.cnt() > 0);
            REQUIRE(forecast.cnt() < 50);

            REQUIRE(forecast.list().size() == forecast.cnt());
            REQUIRE(forecast.list().size() >= 1);

            for(int i = 0; i < forecast.list().size(); ++i)
            {
                // Take a point in the forecast.  This test is based on live
                // weather data, so data cannot be predicted fully.
                aether::openweathermap::point point(forecast.list()[i]);

                // Temperatures are in degrees Kelvin (273 Kelvin == 0 Celsius).
                REQUIRE(point.temp() > 253);
                REQUIRE(point.temp() < 313);
                REQUIRE(point.temp_min() > 253);
                REQUIRE(point.temp_min() < 313);
                REQUIRE(point.temp_max() > 253);
                REQUIRE(point.temp_max() < 313);

                // TODO: figure out what these values mean.
                //REQUIRE(point.temp_min() <= point.temp());
                //REQUIRE(point.temp() <= point.temp_max());

                // Percentage scales.
                REQUIRE(point.humidity() >= 0);
                REQUIRE(point.humidity() <= 100);
                REQUIRE(point.clouds() >= 0);
                REQUIRE(point.clouds() <= 100);

                // Wind speed shouldn't be greater than 100 knots.
                REQUIRE(point.wind_speed() >= 0);
                REQUIRE(point.wind_speed() <= 100);
                REQUIRE(point.wind_deg() >= 0);
                REQUIRE(point.wind_deg() < 360);

                // Time should be within five days of the present time.
                boost::posix_time::ptime point_time =
                    boost::posix_time::from_time_t(point.dt());

                boost::posix_time::ptime now =
                    boost::posix_time::second_clock::universal_time();
                boost::posix_time::ptime start =
                    now - boost::gregorian::days(1);
                boost::posix_time::ptime end =
                    now + boost::gregorian::days(5);

                REQUIRE(point_time >= start);
                REQUIRE(point_time <= end);
            }
        }
    }
}


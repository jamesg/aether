#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "catch.hpp"

#include "atlas/http/client.hpp"
#include "atlas/log/log.hpp"
#include "styx/element.hpp"
#include "styx/styx.hpp"

/*
 * Test the retrieval of weather forecast data from the Open Weather Map
 * service.  Both the availability and format of the data are tested.  As the
 * test case operates on live data, values are only checked within a range of
 * realistic weather conditions.
 */
SCENARIO("openweathermap::forecast") {
    WHEN("a forecast request is made") {
        styx::object forecast;

        boost::shared_ptr<boost::asio::io_service> io(
                new boost::asio::io_service
                );
        atlas::http::get_json(
                io,
                "http://api.openweathermap.org/data/2.5/forecast?lon=51&lat=0",
                [&forecast](styx::element forecast_)
                {
                    forecast = styx::object(forecast_);
                    atlas::log::test("aether::openweathermap::forecast test") <<
                        "retrieved forecast";
                },
                [](const std::string& error) {
                    atlas::log::test("aether::openweathermap::forecast test") <<
                        "error " << error;
                }
                );
        io->run();

        THEN("the result contains sensible data") {
            REQUIRE(forecast.get_int("cnt") > 0);
            REQUIRE(forecast.get_int("cnt") < 50);

            REQUIRE(forecast.get_list("list").size() == forecast.copy_int("cnt"));
            REQUIRE(forecast.get_list("list").size() >= 1);

            for(styx::element point_e : forecast.get_list("list"))
            {
                styx::object point(point_e);
                // Take a point in the forecast.  This test is based on live
                // weather data, so data cannot be predicted fully.

                // Temperatures are in degrees Kelvin (273 Kelvin == 0 Celsius).
                REQUIRE(point.get_object("main").copy_int("temp") > 253);
                REQUIRE(point.get_object("main").copy_int("temp") < 313);
                REQUIRE(point.get_object("main").copy_int("temp_min") > 253);
                REQUIRE(point.get_object("main").copy_int("temp_min") < 313);
                REQUIRE(point.get_object("main").copy_int("temp_max") > 253);
                REQUIRE(point.get_object("main").copy_int("temp_max") < 313);

                // Percentage scales.
                REQUIRE(point.get_object("main").copy_int("humidity") >= 0);
                REQUIRE(point.get_object("main").copy_int("humidity") <= 100);

                // Wind speed shouldn't be greater than 100 knots.
                REQUIRE(point.get_object("wind").copy_int("speed") >= 0);
                REQUIRE(point.get_object("wind").copy_int("speed") <= 100);
                REQUIRE(point.get_object("wind").copy_int("deg") >= 0);
                REQUIRE(point.get_object("wind").copy_int("deg") < 360);

                // Time should be within five days of the present time.
                boost::posix_time::ptime point_time =
                    boost::posix_time::from_time_t(point.copy_int("dt"));

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

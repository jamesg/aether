#include "catch.hpp"

#include "aether/db.hpp"
#include "atlas/log/log.hpp"
#include "styx/element.hpp"
#include "styx/serialise_json.hpp"

#include "openweathermap/city_to_location.hpp"

SCENARIO("openweathermap::city_to_location") {
    WHEN("the location of London is requested") {
        aether::location l;
        boost::shared_ptr<boost::asio::io_service> io(
                new boost::asio::io_service
                );
        aether::openweathermap::city_to_location(
                io,
                "London, UK",
                [&l](aether::location e)
                {
                    l = e;
                    atlas::log::test("aether::openweathermap::city_to_location test") <<
                        "retrieved location: " <<
                        styx::serialise_json(e);
                },
                [](const std::string& error) {
                    atlas::log::test("aether::openweathermap::city_to_location test") <<
                        "error " << error;
                }
                );
        io->run();
        THEN("the location is correct") {
            float lat_cmp = l.get_double<aether::attr::location_lat>()/51.50853;
            REQUIRE(lat_cmp > 0.99);
            REQUIRE(lat_cmp < 1.01);
            float lon_cmp = l.get_double<aether::attr::location_lon>()/-0.12574;
            REQUIRE(lon_cmp > 0.99);
            REQUIRE(lon_cmp < 1.01);
        }
    }

    WHEN("an unknown location is requested") {
        bool received = false;
        boost::shared_ptr<boost::asio::io_service> io(
                new boost::asio::io_service
                );
        aether::openweathermap::city_to_location(
                io,
                "notalocation",
                [](aether::location&) {
                },
                [&received](const std::string& error) {
                    received = true;
                }
                );
        io->run();

        THEN("the error callback was called") {
            REQUIRE(received);
        }
    }
}

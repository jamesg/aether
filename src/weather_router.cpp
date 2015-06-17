#include "weather_router.hpp"

#include "aether/db.hpp"
#include "hades/custom_select_one.hpp"
#include "hades/join.hpp"

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
                        hades::order_by("aether_forecast.forecast_dt", 1)
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
                    hades::order_by("aether_forecast.forecast_dt", 1)
                    )
                );
            return atlas::http::json_response(points.at(0));
        }
        );
}


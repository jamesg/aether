#include "aether/router.hpp"

#include <boost/bind.hpp>

#include "aether/db.hpp"
#include "atlas/auth/router.hpp"
#include "atlas/db/date.hpp"
#include "atlas/http/server/exception.hpp"
#include "atlas/http/server/static_string.hpp"
#include "hades/crud.ipp"
#include "hades/custom_select.hpp"
#include "hades/exists.hpp"
#include "hades/join.hpp"

#include "kb_router.hpp"
#include "openweathermap/city_to_location.hpp"
#include "sensor_api.hpp"
#include "weather_router.hpp"

#define AETHER_DECLARE_STATIC_STRING(NAME) ATLAS_DECLARE_STATIC_STRING(aether, NAME)
#define AETHER_STATIC_STD_STRING(NAME) ATLAS_STATIC_STD_STRING(aether, NAME)

AETHER_DECLARE_STATIC_STRING(aether_css)
AETHER_DECLARE_STATIC_STRING(application_js)
AETHER_DECLARE_STATIC_STRING(board_html)
AETHER_DECLARE_STATIC_STRING(board_js)
AETHER_DECLARE_STATIC_STRING(chart_js)
AETHER_DECLARE_STATIC_STRING(index_html)
AETHER_DECLARE_STATIC_STRING(index_js)
AETHER_DECLARE_STATIC_STRING(knowledgebase_editor_html)
AETHER_DECLARE_STATIC_STRING(knowledgebase_editor_js)
AETHER_DECLARE_STATIC_STRING(knowledgebase_html)
AETHER_DECLARE_STATIC_STRING(knowledgebase_js)
AETHER_DECLARE_STATIC_STRING(models_js)
AETHER_DECLARE_STATIC_STRING(palette_html)
AETHER_DECLARE_STATIC_STRING(palette_js)
AETHER_DECLARE_STATIC_STRING(settings_html)
AETHER_DECLARE_STATIC_STRING(settings_js)
AETHER_DECLARE_STATIC_STRING(weather_html)
AETHER_DECLARE_STATIC_STRING(weather_js)

namespace
{
    static constexpr char sowing[] = "sowing";
}

aether::router::router(
        boost::shared_ptr<boost::asio::io_service> io,
        hades::connection& conn
    ) :
    atlas::http::application_router(io)
{
    install_static_text("/", "html", AETHER_STATIC_STD_STRING(index_html));

    install_static_text("/aether.css", AETHER_STATIC_STD_STRING(aether_css));
    install_static_text("/application.js", AETHER_STATIC_STD_STRING(application_js));
    install_static_text("/board.html", AETHER_STATIC_STD_STRING(board_html));
    install_static_text("/board.js", AETHER_STATIC_STD_STRING(board_js));
    install_static_text("/chart.js", AETHER_STATIC_STD_STRING(chart_js));
    install_static_text("/index.html", AETHER_STATIC_STD_STRING(index_html));
    install_static_text("/index.js", AETHER_STATIC_STD_STRING(index_js));
    install_static_text("/knowledgebase.html", AETHER_STATIC_STD_STRING(knowledgebase_html));
    install_static_text("/knowledgebase.js", AETHER_STATIC_STD_STRING(knowledgebase_js));
    install_static_text("/knowledgebase_editor.html", AETHER_STATIC_STD_STRING(knowledgebase_editor_html));
    install_static_text("/knowledgebase_editor.js", AETHER_STATIC_STD_STRING(knowledgebase_editor_js));
    install_static_text("/models.js", AETHER_STATIC_STD_STRING(models_js));
    install_static_text("/palette.html", AETHER_STATIC_STD_STRING(palette_html));
    install_static_text("/palette.js", AETHER_STATIC_STD_STRING(palette_js));
    install_static_text("/settings.html", AETHER_STATIC_STD_STRING(settings_html));
    install_static_text("/settings.js", AETHER_STATIC_STD_STRING(settings_js));
    install_static_text("/weather.html", AETHER_STATIC_STD_STRING(weather_html));
    install_static_text("/weather.js", AETHER_STATIC_STD_STRING(weather_js));

    boost::shared_ptr<atlas::http::router> kbr(new kb_router(io, conn));
    install(atlas::http::matcher("/api/kb(.*)", 1), kbr);

    boost::shared_ptr<atlas::http::router> auth(new atlas::auth::router(io, conn));
    install(atlas::http::matcher("/api/auth(.*)", 1), auth);

    boost::shared_ptr<sensor_api> sapi(new sensor_api(io, conn));
    install(
        atlas::http::matcher("/rpc/sensor"),
        boost::bind(&atlas::api::server::serve, sapi, _1, _2, _3, _4)
        );

    boost::shared_ptr<atlas::http::router> weather_r(new weather_router(io, conn));
    install(atlas::http::matcher("/api/weather(.*)", 1), weather_r);

    //
    // Batches.
    //

    // Get all batches.
    install<>(
        atlas::http::matcher("/api/batch", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::outer_join<
                    batch,
                    sensor_at_batch,
                    sensor,
                    batch_phase,
                    phase,
                    kb::variety,
                    kb::family,
                    batch_sowing,
                    batch_planting>(
                    conn,
                    "aether_batch.batch_id = aether_sensor_at_batch.sensor_id",
                    "aether_sensor_at_batch.sensor_id = aether_sensor.sensor_id",
                    "aether_batch.batch_id = aether_batch_phase.phase_id",
                    "aether_batch_phase.phase_id = aether_phase.phase_id",
                    "aether_batch.kb_variety_id = aether_kb_variety.kb_variety_id",
                    "aether_kb_variety.kb_variety_id = aether_kb_family.kb_family_id",
                    "aether_batch.batch_id = aether_batch_sowing.batch_id",
                    "aether_batch.batch_id = aether_batch_planting.batch_id"
                )
            );
        }
    );

    // Get all batches ordered by the date they entered their current phase,
    // descending.
    install<>(
        atlas::http::matcher("/api/batch/recently_moved", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::join<
                    batch,
                    batch_phase,
                    phase,
                    sensor_at_batch,
                    kb::variety,
                    kb::family>(
                        conn,
                        hades::filter(
                            hades::where(
                                "aether_batch.batch_id = aether_batch_phase.batch_id AND "
                                "aether_batch_phase.phase_id = aether_phase.phase_id AND "
                                "aether_batch.kb_variety_id = aether_kb_variety.kb_variety_id AND "
                                "aether_kb_variety.kb_family_id = aether_kb_family.kb_family_id "
                            ),
                            hades::order_by("aether_batch_phase.start DESC")
                        )
                    )
            );
        }
    );
    // Get a single batch.
    auto get_batch = [&conn](const styx::int_type batch_id) -> atlas::http::response {
        return atlas::http::json_response(
            styx::first(
                hades::outer_join<
                    batch,
                    sensor_at_batch,
                    sensor,
                    batch_phase,
                    phase,
                    kb::variety,
                    kb::family,
                    batch_sowing,
                    batch_planting>(
                    conn,
                    "aether_batch.batch_id = aether_sensor_at_batch.sensor_id",
                    "aether_sensor_at_batch.sensor_id = aether_sensor.sensor_id",
                    "aether_batch.batch_id = aether_batch_phase.phase_id",
                    "aether_batch_phase.phase_id = aether_phase.phase_id",
                    "aether_batch.kb_variety_id = aether_kb_variety.kb_variety_id",
                    "aether_kb_variety.kb_variety_id = aether_kb_family.kb_family_id",
                    "aether_batch.batch_id = aether_batch_sowing.batch_id",
                    "aether_batch.batch_id = aether_batch_planting.batch_id",
                    hades::where(
                        "aether_batch.batch_id = ?",
                        hades::row<styx::int_type>(batch_id)
                    )
                )
            )
        );
    };
    install<styx::int_type>(
        atlas::http::matcher("/api/batch/([0-9]+)", "GET"),
        get_batch
    );
    // Get the history of a batch (all phases it has been in) including the
    // start and finish dates in each phase.  The finish date will be null if
    // the batch is currently in a phase.
    install<styx::int_type>(
        atlas::http::matcher("/api/batch/([0-9]+)/history", "GET"),
        [&conn](const styx::int_type batch_id) {
            return atlas::http::json_response(
                hades::custom_select<
                    batch,
                    hades::row<styx::int_type, styx::int_type>,
                    attr::batch_id, attr::start, attr::finish,
                    attr::phase_id, attr::phase_desc
                    >(
                        conn,
                        "SELECT batch_id, start, NULL AS finish, aether_phase.phase_id AS phase_id, phase_desc "
                        " FROM aether_batch_phase "
                        " JOIN aether_phase "
                        "  ON aether_batch_phase.phase_id = aether_phase.phase_id "
                        " WHERE batch_id = ? "
                        "UNION "
                        "SELECT batch_id, start, finish, aether_phase.phase_id AS phase_id, phase_desc "
                        " FROM aether_batch_phase_history "
                        " JOIN aether_phase "
                        "  ON aether_batch_phase_history.phase_id = aether_phase.phase_id "
                        " WHERE batch_id = ? ",
                        hades::row<styx::int_type, styx::int_type>(batch_id, batch_id)
                    )
                );
        }
    );
    install_json<batch>(
        atlas::http::matcher("/api/batch", "POST"),
        [&conn](batch b) {
            b.insert(conn);
            batch_phase bp;
            bp.set_id(b.id());
            bp.get_int("phase_id") = b.get_int("phase_id");
            bp.get_int<attr::start>() = atlas::db::date::to_unix_time(
                boost::posix_time::second_clock::universal_time()
                );
            bp.insert(conn);

            if(b.has_key("sensor_id"))
            {
                // Remove the sensor from any batch it is currently assigned to.
                hades::devoid(
                    "DELETE FROM aether_sensor_at_batch WHERE "
                    " sensor_id = ? ",
                    hades::row<styx::int_type>(b.get_int("sensor_id")),
                    conn
                );
                sensor_at_batch sab;
                sab.set_id(b.id());
                sab.get_int<attr::sensor_id>() = b.get_int("sensor_id");
                sab.insert(conn);
            }
            return atlas::http::json_response(b);
        },
        [&conn](const atlas::auth::token_type& token) {
            // implies
            return !db::bool_setting(conn, "permission_create_batch", false) ||
                atlas::auth::is_superuser(conn, token);
        }
    );
    // Copy a batch and its history without moving any assigned sensor.  Return
    // the new batch.
    install<styx::int_type>(
        atlas::http::matcher("/api/batch/([0-9]+)/copy", "GET"),
        [&conn, get_batch](const styx::int_type batch_id) {
            batch b(hades::get_by_id<batch>(conn, batch::id_type{batch_id}));
            b.remove_id();
            b.insert(conn);

            styx::list bpl(
                batch_phase::get_collection(
                    conn,
                    hades::where(
                        "batch_id = ?",
                        hades::row<styx::int_type>(batch_id)
                    )
                )
            );
            for(batch_phase bp : bpl)
            {
                bp.get_int<attr::batch_id>() = b.copy_int<attr::batch_id>();
                bp.insert(conn);
            }

            styx::list bphl(
                batch_phase_history::get_collection(
                    conn,
                    hades::where(
                        "batch_id = ?",
                        hades::row<styx::int_type>(batch_id)
                    )
                )
            );
            for(batch_phase_history bph : bpl)
            {
                bph.get_int<attr::batch_id>() = b.copy_int<attr::batch_id>();
                bph.insert(conn);
            }

            return get_batch(b.copy_int<attr::batch_id>());
        }
    );
    install_json<batch, int>(
        atlas::http::matcher("/api/batch/([0-9]+)", "PUT"),
        [&conn](batch b, const int batch_id) {
            try
            {
                batch_phase current_phase(
                    hades::get_by_id<batch_phase>(conn, batch::id_type{batch_id})
                    );
                if(
                    b.has_key(attr::phase_id) &&
                    b.get_int(attr::phase_id) != current_phase.get_int<attr::phase_id>()
                  )
                {
                    // Add the current phase to the history.
                    // 'history' will be constructed with the current time.
                    batch_phase_history history(current_phase);
                    history.insert(conn);
                    // Overwrite the current phase.
                    current_phase.get_int<attr::phase_id>() = b.get_int(attr::phase_id);
                    current_phase.get_int<attr::start>() =
                        atlas::db::date::to_unix_time(
                            boost::posix_time::second_clock::universal_time()
                        );
                    current_phase.update(conn);
                }
                sensor_at_batch sab;
                sab.set_id(b.id());
                if(b.has_key("sensor_id"))
                {
                    // Remove any sensor at this batch.
                    sab.destroy(conn);
                    // Remove the sensor from any batch it is currently assigned
                    // to.
                    hades::devoid(
                        "DELETE FROM aether_sensor_at_batch WHERE "
                        " sensor_id = ? ",
                        hades::row<styx::int_type>(b.get_int("sensor_id")),
                        conn
                    );
                    sab.get_int<attr::sensor_id>() = b.get_int("sensor_id");
                    sab.insert(conn);
                }
                else
                {
                    sab.destroy(conn);
                }
            }
            catch(const std::exception& e)
            {
                atlas::log::warning("aether::router") << "update batch without phase_id";
            }

            b.update(conn);
            return atlas::http::json_response(b);
        },
        [&conn](const atlas::auth::token_type& token, int) {
            // implies
            return !db::bool_setting(conn, "permission_move_batch", false) ||
                atlas::auth::is_superuser(conn, token);
        }
    );

    //
    // Phases.
    //

    install<>(
        atlas::http::matcher("/api/phase", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::equi_outer_join<phase, phase_order, phase_temperature>(
                    conn,
                    hades::order_by("aether_phase_order.phase_order ASC")
                )
            );
        }
        );
    install_json<styx::list>(
        atlas::http::matcher("/api/phase", "PUT"),
        [&conn](const styx::list& l) {
            phase::save_collection(l, conn);
            phase_order::overwrite_collection(l, conn);
            return atlas::http::json_response(l);
        }
    );
    install<int>(
        atlas::http::matcher("/api/phase/([0-9]+)", "DELETE"),
        [&conn](const int phase_id) {
            // TODO reassign batches to 'Unknown' phase.
            phase::id_type id{phase_id};
            phase_order po;
            po.set_id(id);
            po.destroy(conn);
            phase p;
            p.set_id(phase::id_type{po});
            return atlas::http::json_response(p.destroy(conn));
        }
    );
    install<int>(
        atlas::http::matcher("/api/phase/([0-9]+)", "GET"),
        [&conn](const int phase_id) {
            return atlas::http::json_response(
                hades::get_by_id<phase>(conn, phase::id_type{phase_id})
                );
        }
    );
    install<styx::int_type>(
        atlas::http::matcher("/api/phase/([0-9]+)/temperature", "GET"),
        [&conn](const styx::int_type phase_id) {
            return atlas::http::json_response(
                styx::first(
                    hades::join<phase, temperature_log>(
                        conn,
                        hades::filter(
                            hades::where(
                                "aether_phase.phase_id = "
                                " aether_temperature_log.phase_id AND "
                                "aether_phase.phase_id = ?",
                                hades::row<styx::int_type>(phase_id)
                            ),
                            hades::order_by("log_time DESC", 1)
                        )
                    )
                )
            );
        }
    );
    install_json<phase>(
        atlas::http::matcher("/api/phase", "POST"),
        [&conn](phase p) {
            p.insert(conn);
            return atlas::http::json_response(p);
        }
    );
    install_json<phase, int>(
        atlas::http::matcher("/api/phase/([0-9]+)", "PUT"),
        [&conn](phase p, const int phase_id) {
            p.update(conn);
            return atlas::http::json_response(p);
        }
    );

    //
    // Sensors.
    //

    install<>(
        atlas::http::matcher("/api/sensor", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::equi_outer_join<
                    sensor,
                    hades::flag<sensor::id_type, flag::moisture_sensor>,
                    hades::flag<sensor::id_type, flag::temperature_sensor>
                    >(conn)
                );
        }
    );
    install<int>(
        atlas::http::matcher("/api/sensor/([0-9]+)", "DELETE"),
        [&conn](const int sensor_id) {
            sensor s;
            s.set_id(sensor::id_type{sensor_id});
            return atlas::http::json_response(s.destroy(conn));
        }
    );
    install<int>(
        atlas::http::matcher("/api/sensor/([0-9]+)", "GET"),
        [&conn](const int sensor_id) {
            return atlas::http::json_response(
                hades::get_by_id<sensor>(
                    conn,
                    sensor::id_type{sensor_id}
                )
            );
        }
    );
    install_json<sensor>(
        atlas::http::matcher("/api/sensor", "POST"),
        [&conn](sensor s) {
            s.insert(conn);
            return atlas::http::json_response(s);
        }
    );
    install_json<sensor, int>(
        atlas::http::matcher("/api/sensor/([0-9]+)", "PUT"),
        [&conn](sensor s, const int sensor_id) {
            s.update(conn);
            return atlas::http::json_response(s);
        }
    );

    //
    // Location.
    //

    install_json_async<location>(
        atlas::http::matcher("/api/location/search", "POST"),
        [io](
            location l,
            atlas::http::uri_success_callback_type success,
            atlas::http::uri_callback_type error
            )
        {
            openweathermap::city_to_location(
                io,
                l.get_string<attr::location_city>(),
                [success](location l) {
                    success(atlas::http::json_response(l));
                },
                [error](std::string) {
                    error();
                }
                );
        }
    );
    install_json<location>(
        atlas::http::matcher("/api/location", "DELETE"),
        [&conn](location l) {
            return atlas::http::json_response(l.destroy(conn));
        }
    );
    install<>(
        atlas::http::matcher("/api/location", "GET"),
        [&conn]() {
            if(!hades::exists<location>(conn))
                throw atlas::http::exception("not found", 404);
            return atlas::http::json_response(hades::get_one<location>(conn));
        }
    );
    install_json<location>(
        atlas::http::matcher("/api/location", "POST"),
        [&conn](location l) {
            l.save(conn);
            return atlas::http::json_response(l);
        }
    );
    install_json<location>(
        atlas::http::matcher("/api/location", "PUT"),
        [&conn](location l) {
            l.save(conn);
            return atlas::http::json_response(l);
        }
    );

    //
    // Settings
    //

    install<>(
        atlas::http::matcher("/api/settings", "GET"),
        [&conn]() {
            return atlas::http::json_response(db::settings(conn));
        }
    );
    install_json<styx::object>(
        atlas::http::matcher("/api/settings", "POST"),
        [&conn](styx::object new_settings) {
            db::save_settings(new_settings, conn);
            return atlas::http::json_response(db::settings(conn));
        }
    );
    install_json<styx::object>(
        atlas::http::matcher("/api/settings", "PUT"),
        [&conn](styx::object new_settings) {
            db::save_settings(new_settings, conn);
            return atlas::http::json_response(db::settings(conn));
        }
    );
}

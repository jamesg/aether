#include "aether/router.hpp"

#include <boost/bind.hpp>

#include "aether/db.hpp"
#include "atlas/http/server/static_string.hpp"
#include "hades/crud.ipp"
#include "hades/join.hpp"

#include "kb_router.hpp"

#define AETHER_DECLARE_STATIC_STRING(NAME) ATLAS_DECLARE_STATIC_STRING(aether, NAME)
#define AETHER_STATIC_STD_STRING(NAME) ATLAS_STATIC_STD_STRING(aether, NAME)

AETHER_DECLARE_STATIC_STRING(aether_css)
AETHER_DECLARE_STATIC_STRING(application_js)
AETHER_DECLARE_STATIC_STRING(board_html)
AETHER_DECLARE_STATIC_STRING(board_js)
AETHER_DECLARE_STATIC_STRING(index_html)
AETHER_DECLARE_STATIC_STRING(index_js)
AETHER_DECLARE_STATIC_STRING(knowledgebase_editor_html)
AETHER_DECLARE_STATIC_STRING(knowledgebase_editor_js)
AETHER_DECLARE_STATIC_STRING(knowledgebase_html)
AETHER_DECLARE_STATIC_STRING(knowledgebase_js)
AETHER_DECLARE_STATIC_STRING(models_js)
AETHER_DECLARE_STATIC_STRING(sensors_html)
AETHER_DECLARE_STATIC_STRING(sensors_js)
AETHER_DECLARE_STATIC_STRING(settings_html)
AETHER_DECLARE_STATIC_STRING(settings_js)

aether::router::router(hades::connection& conn)
{
    install_static_text("/", "html", AETHER_STATIC_STD_STRING(index_html));

    install_static_text("/aether.css", AETHER_STATIC_STD_STRING(aether_css));
    install_static_text("/application.js", AETHER_STATIC_STD_STRING(application_js));
    install_static_text("/board.html", AETHER_STATIC_STD_STRING(board_html));
    install_static_text("/board.js", AETHER_STATIC_STD_STRING(board_js));
    install_static_text("/index.html", AETHER_STATIC_STD_STRING(index_html));
    install_static_text("/index.js", AETHER_STATIC_STD_STRING(index_js));
    install_static_text("/knowledgebase.html", AETHER_STATIC_STD_STRING(knowledgebase_html));
    install_static_text("/knowledgebase.js", AETHER_STATIC_STD_STRING(knowledgebase_js));
    install_static_text("/knowledgebase_editor.html", AETHER_STATIC_STD_STRING(knowledgebase_editor_html));
    install_static_text("/knowledgebase_editor.js", AETHER_STATIC_STD_STRING(knowledgebase_editor_js));
    install_static_text("/models.js", AETHER_STATIC_STD_STRING(models_js));
    install_static_text("/sensors.html", AETHER_STATIC_STD_STRING(sensors_html));
    install_static_text("/sensors.js", AETHER_STATIC_STD_STRING(sensors_js));
    install_static_text("/settings.html", AETHER_STATIC_STD_STRING(settings_html));
    install_static_text("/settings.js", AETHER_STATIC_STD_STRING(settings_js));

    boost::shared_ptr<atlas::http::router> kbr(new kb_router(conn));
    install(
        atlas::http::matcher("/api/kb(.*)", 1),
        boost::bind(&atlas::http::router::serve, kbr, _1, _2, _3, _4)
        );

    //
    // Batches.
    //

    install<>(
        atlas::http::matcher("/api/batch", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::outer_join<
                    batch,
                    batch_phase,
                    kb::family,
                    kb::variety>(
                        conn,
                        "aether_batch.batch_id = aether_batch_phase.batch_id AND "
                        "aether_batch.kb_variety_id = aether_kb_variety.kb_variety_id AND "
                        "aether_kb_variety.kb_family_id = aether_kb_family.kb_family_id "
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
            bp.insert(conn);
            return atlas::http::json_response(b);
        }
        );
    install_json<batch, int>(
        atlas::http::matcher("/api/batch/([0-9]+)", "PUT"),
        [&conn](batch b, const int) {
            b.update(conn);
            return atlas::http::json_response(b);
            }
        );
    install<int>(
        atlas::http::matcher("/api/phase/([0-9]+)/batch", "GET"),
        [&conn](const int phase_id) {
            return atlas::http::json_response(
                hades::outer_join<
                    batch,
                    batch_phase,
                    kb::variety,
                    kb::family>(
                        conn,
                        "aether_batch.batch_id = aether_batch_phase.batch_id AND "
                        "aether_batch.kb_variety_id = aether_kb_variety.kb_variety_id AND "
                        "aether_kb_variety.kb_family_id = aether_kb_family.kb_family_id ",
                        hades::where(
                            "aether_batch_phase.phase_id = ? ",
                            hades::row<int>(phase_id)
                            )
                        )
                );
        }
        );

    //
    // Phases.
    //

    install<>(
        atlas::http::matcher("/api/phase", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::equi_outer_join<phase, phase_order>(
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
}


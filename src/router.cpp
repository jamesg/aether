#include "aether/router.hpp"

#include "aether/db.hpp"
#include "atlas/http/server/static_string.hpp"
#include "hades/crud.ipp"
#include "hades/join.hpp"

#define AETHER_DECLARE_STATIC_STRING(NAME) ATLAS_DECLARE_STATIC_STRING(aether, NAME)
#define AETHER_STATIC_STD_STRING(NAME) ATLAS_STATIC_STD_STRING(aether, NAME)

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

    //
    // Batches.
    //

    install<>(
        atlas::http::matcher("/api/batch", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::equi_outer_join<batch, batch_phase>(conn)
                );
        }
        );
    install_json<batch>(
        atlas::http::matcher("/api/batch", "POST"),
        [&conn](batch b) {
            b.insert(conn);
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
                hades::equi_outer_join<batch, batch_phase>(
                    conn,
                    hades::where(
                        "aether_batch_phase.phase_id = ?",
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
                hades::get_by_id<phase>(conn, phase::id_type(phase_id))
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
    // Knowledge Base.
    //

    install<int>(
        atlas::http::matcher("/api/kb/family/([0-9]+)", "DELETE"),
        [&conn](const int family_id) {
            kb::family f;
            f.set_id(kb::family::id_type{family_id});
            // TODO reassign varieties
            return atlas::http::json_response(f.destroy(conn));
        }
        );
    install<>(
        atlas::http::matcher("/api/kb/family", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::get_collection<kb::family>(conn)
                );
        }
        );
    install_json<kb::family>(
        atlas::http::matcher("/api/kb/family", "POST"),
        [&conn](kb::family f) {
            f.insert(conn);
            return atlas::http::json_response(f);
        }
        );
    install<int>(
        atlas::http::matcher("/api/kb/variety/([0-9]+)", "DELETE"),
        [&conn](const int variety_id) {
            kb::variety v;
            v.set_id(kb::variety::id_type{variety_id});
            // TODO reassign batches
            return atlas::http::json_response(v.destroy(conn));
        }
        );
    install<>(
        atlas::http::matcher("/api/kb/variety", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::get_collection<kb::variety>(conn)
                );
        }
        );
    install_json<kb::variety>(
        atlas::http::matcher("/api/kb/variety", "POST"),
        [&conn](kb::variety v) {
            v.insert(conn);
            return atlas::http::json_response(v);
        }
        );
    install<int>(
        atlas::http::matcher("/api/kb/family/([0-9]+)/variety", "GET"),
        [&conn](const int family_id) {
            return atlas::http::json_response(
                hades::get_collection<kb::variety>(
                    conn,
                    hades::where(
                        "aether_kb_variety.family_id = ?",
                        hades::row<int>(family_id)
                        )
                    )
                );
        }
        );
}


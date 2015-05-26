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
    install_json<styx::element>(
        atlas::http::matcher("/api/phase", "POST"),
        [&conn](const styx::element& e) {
            phase p(e);
            p.insert(conn);
            return atlas::http::json_response(p);
        }
        );
    install_json<styx::element, int>(
        atlas::http::matcher("/api/phase/([0-9]+)", "PUT"),
        [&conn](const styx::element& e, const int phase_id) {
            phase p(e);
            p.update(conn);
            return atlas::http::json_response(p);
        }
        );
}


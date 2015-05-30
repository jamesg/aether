#include "kb_router.hpp"

#include "aether/db.hpp"
#include "hades/crud.ipp"
#include "hades/join.hpp"
#include "styx/cast.hpp"

aether::kb_router::kb_router(hades::connection& conn)
{
    //
    // Knowledge Base.
    //

    install<int>(
        atlas::http::matcher("/family/([0-9]+)", "DELETE"),
        [&conn](const int family_id) {
            kb::family f;
            f.set_id(kb::family::id_type{family_id});
            // TODO reassign varieties
            return atlas::http::json_response(f.destroy(conn));
        }
        );
    install_json<kb::family, int>(
        atlas::http::matcher("/family/([0-9]+)", "PUT"),
        [&conn](kb::family f, const int family_id) {
            f.update(conn);
            return atlas::http::json_response(f);
        }
        );
    install<>(
        atlas::http::matcher("/family", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::get_collection<kb::family>(conn)
                );
        }
        );
    install_json<kb::family>(
        atlas::http::matcher("/family", "POST"),
        [&conn](kb::family f) {
            f.insert(conn);
            return atlas::http::json_response(f);
        }
        );
    install<int>(
        atlas::http::matcher("/variety/([0-9]+)", "DELETE"),
        [&conn](const int variety_id) {
            kb::variety v;
            v.set_id(kb::variety::id_type{variety_id});
            // TODO reassign batches
            return atlas::http::json_response(v.destroy(conn));
        }
        );
    install<int>(
        atlas::http::matcher("/variety/([0-9]+)", "GET"),
        [&conn](const int variety_id) {
            kb::variety v = hades::get_by_id<kb::variety>(
                conn,
                kb::variety::id_type{variety_id}
                );

            styx::list harvest_mon = kb::variety_harvest_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_harvest_mon.kb_variety_id = ?",
                    hades::row<int>(variety_id)
                    )
                );
            for(kb::variety_harvest_mon mon : harvest_mon)
                v.get_list("harvest_mon").append(
                    mon.get_int<attr::kb_variety_harvest_mon_month>()
                    );
            styx::list plant_mon = kb::variety_plant_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_plant_mon.kb_variety_id = ?",
                    hades::row<int>(variety_id)
                    )
                );
            for(kb::variety_plant_mon mon : plant_mon)
                v.get_list("plant_mon").append(
                    mon.get_int<attr::kb_variety_plant_mon_month>()
                    );
            styx::list sow_mon = kb::variety_sow_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_sow_mon.kb_variety_id = ?",
                    hades::row<int>(variety_id)
                    )
                );
            for(kb::variety_sow_mon mon : sow_mon)
                v.get_list("sow_mon").append(
                    mon.get_int<attr::kb_variety_sow_mon_month>()
                    );
            return atlas::http::json_response(v);
        }
        );
    // TODO accept a const &
    auto save_months = [&conn](kb::variety v) {
        if(v.has_key("harvest_mon"))
        {
            styx::list months;
            for(const styx::element& e : v.get_list("harvest_mon"))
                months.append(
                    kb::variety_harvest_mon::id_type{
                        v.copy_int<attr::kb_variety_id>(),
                        styx::cast<int>(e)
                    }
                    );
            kb::variety_harvest_mon::overwrite_collection(
                months,
                hades::where(
                    "aether_kb_variety_harvest_mon.kb_variety_id = ?",
                    hades::row<int>(v.copy_int<attr::kb_variety_id>())
                    ),
                conn
                );
        }
        if(v.has_key("plant_mon"))
        {
            styx::list months;
            for(const styx::element& e : v.get_list("plant_mon"))
                months.append(
                    kb::variety_plant_mon::id_type{
                        v.copy_int<attr::kb_variety_id>(),
                        styx::cast<int>(e)
                    }
                    );
            kb::variety_plant_mon::overwrite_collection(
                months,
                hades::where(
                    "aether_kb_variety_plant_mon.kb_variety_id = ?",
                    hades::row<int>(v.copy_int<attr::kb_variety_id>())
                    ),
                conn
                );
        }
        if(v.has_key("sow_mon"))
        {
            styx::list months;
            for(const styx::element& e : v.get_list("sow_mon"))
                months.append(
                    kb::variety_sow_mon::id_type{
                        v.copy_int<attr::kb_variety_id>(),
                        styx::cast<int>(e)
                    }
                    );
            kb::variety_sow_mon::overwrite_collection(
                months,
                hades::where(
                    "aether_kb_variety_sow_mon.kb_variety_id = ?",
                    hades::row<int>(v.copy_int<attr::kb_variety_id>())
                    ),
                conn
                );
        }
    };
    install_json<kb::variety, int>(
        atlas::http::matcher("/variety/([0-9]+)", "PUT"),
        [&conn, save_months](kb::variety v, const int variety_id) {
            v.update(conn);
            save_months(v);
            return atlas::http::json_response(v);
        }
        );
    install<>(
        atlas::http::matcher("/variety", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::get_collection<kb::variety>(conn)
                );
        }
        );
    install_json<kb::variety>(
        atlas::http::matcher("/variety", "POST"),
        [&conn, save_months](kb::variety v) {
            v.insert(conn);
            save_months(v);
            return atlas::http::json_response(v);
        }
        );
    install<int>(
        atlas::http::matcher("/family/([0-9]+)/variety", "GET"),
        [&conn](const int family_id) {
            return atlas::http::json_response(
                hades::get_collection<kb::variety>(
                    conn,
                    hades::where(
                        "aether_kb_variety.kb_family_id = ?",
                        hades::row<int>(family_id)
                        )
                    )
                );
        }
        );

}


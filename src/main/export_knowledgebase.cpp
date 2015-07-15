#include "export_knowledgebase.hpp"

#include <iostream>

#include "aether/db.hpp"
#include "commandline/commandline.hpp"
#include "hades/connection.hpp"
#include "hades/crud.ipp"
#include "styx/serialise_json.hpp"

void aether::export_knowledgebase(hades::connection& conn)
{
    styx::list families(kb::family::get_collection(conn));
    for(styx::element& e : families)
    {
        styx::object& family = boost::get<styx::object>(e);
        family.get_list("varieties") = kb::variety::get_collection(
            conn,
            hades::where(
                "aether_kb_variety.kb_family_id = ?",
                hades::row<styx::int_type>(family.get_int("kb_family_id"))
            )
        );
        for(styx::element& e : family.get_list("varieties"))
        {
            styx::object& v = boost::get<styx::object>(e);
            styx::int_type variety_id = v.get_int(attr::kb_variety_id);

            v.get_list("harvest_mon") = kb::variety_harvest_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_harvest_mon.kb_variety_id = ?",
                    hades::row<styx::int_type>(variety_id)
                )
            );
            v.get_list("plant_mon") = kb::variety_plant_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_plant_mon.kb_variety_id = ?",
                    hades::row<styx::int_type>(variety_id)
                )
            );
            v.get_list("sow_mon") = kb::variety_sow_mon::get_collection(
                conn,
                hades::where(
                    "aether_kb_variety_sow_mon.kb_variety_id = ?",
                    hades::row<styx::int_type>(variety_id)
                )
            );
        }
    }
    std::cout << styx::serialise_json(families) << std::endl;
}

int main(const int argc, const char *argv[])
{
    std::string dbfile;
    std::vector<commandline::option> options{
        commandline::parameter("db", dbfile)
    };
    commandline::parse(argc, argv, options);

    if(dbfile == "")
    {
        std::cerr << "dbfile must be specified" << std::endl;
        commandline::print(argc, argv, options);
        return 1;
    }
    hades::connection conn(dbfile);

    try
    {
        aether::export_knowledgebase(conn);
    }
    catch(const std::exception& e)
    {
        std::cerr << "error exporting knowledgebase: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

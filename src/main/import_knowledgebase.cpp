#include "import_knowledgebase.hpp"

#include <iostream>

#include "aether/db.hpp"
#include "commandline/commandline.hpp"
#include "hades/connection.hpp"
#include "hades/crud.ipp"
#include "styx/serialise_json.hpp"

void aether::import_knowledgebase(styx::element kb_e, hades::connection& conn)
{
    std::cerr << styx::serialise_json(kb_e) << std::endl;
    styx::list& families = boost::get<styx::list>(kb_e);
    for(styx::element& e : families)
    {
        kb::family f(e);
        f.save(conn);
        for(styx::element& e : f.get_list("varieties"))
        {
            kb::variety v(e);
            v.save(conn);
        }
    }
}

int main(const int argc, const char *argv[])
{
    std::string dbfile, infile;
    std::vector<commandline::option> options{
        commandline::parameter("db", dbfile),
        commandline::parameter("data", infile)
    };
    commandline::parse(argc, argv, options);

    if(dbfile == "")
    {
        std::cerr << "dbfile must be specified" << std::endl;
        commandline::print(argc, argv, options);
        return 1;
    }
    hades::connection conn(dbfile);
    aether::db::create(conn);

    styx::element kb_e;
    if(infile == "")
        kb_e = styx::parse_json(std::cin);
    else
        kb_e = styx::parse_json_file(infile);
    aether::import_knowledgebase(kb_e, conn);
    return 0;
}

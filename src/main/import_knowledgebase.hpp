#ifndef AETHER_MAIN_IMPORT_KNOWLEDGEBASE_HPP
#define AETHER_MAIN_IMPORT_KNOWLEDGEBASE_HPP

#include "styx/element.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    void import_knowledgebase(styx::element, hades::connection&);
}

#endif

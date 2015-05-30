#ifndef AETHER_KB_ROUTER_HPP
#define AETHER_KB_ROUTER_HPP

#include "atlas/http/server/router.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    /*!
     * \brief Router to serve all requests for /api/kb.  These include all
     * knowledge base types.
     */
    class kb_router : public atlas::http::router
    {
    public:
        explicit kb_router(hades::connection& conn);
    };
}

#endif


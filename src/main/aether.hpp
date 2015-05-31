#ifndef AETHER_MAIN_AETHER_HPP
#define AETHER_MAIN_AETHER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "aether/router.hpp"
#include "atlas/http/server/mimetypes.hpp"
#include "atlas/http/server/server.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    class radio_server;

    class server
    {
    public:
        struct options {
            std::string address, db, port;
            options() :
                address("0.0.0.0")
            {
            }
        };

        server(const options&, boost::shared_ptr<boost::asio::io_service>);
        void start();
        void stop();
    private:
        boost::shared_ptr<boost::asio::io_service> m_io;
        boost::shared_ptr<hades::connection> m_connection;
        boost::shared_ptr<atlas::http::router> m_router;
        boost::shared_ptr<atlas::http::server> m_http_server;
        boost::shared_ptr<aether::radio_server> m_radio_server;
        atlas::http::mimetypes m_mime_information;
    };
}

#endif


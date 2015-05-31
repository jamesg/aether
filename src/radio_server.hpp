#ifndef AETHER_RADIO_SERVER_HPP
#define AETHER_RADIO_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "atlas/api/server.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    class radio_server :
        public boost::enable_shared_from_this<radio_server>,
        atlas::api::server
    {
    public:
        radio_server(
                boost::shared_ptr<boost::asio::io_service> io,
                hades::connection& conn
                );
        void start();
    private:
        void async_read();
        void async_read_received(
                const boost::system::error_code&
                );
        void send_result(const atlas::jsonrpc::result&);
        void send_result_complete(const boost::system::error_code& ec);

        boost::shared_ptr<boost::asio::io_service> m_io;
        boost::asio::serial_port m_port;
        boost::asio::streambuf m_buffer;
    };
}

#endif


#include "radio_server.hpp"

#include <istream>

#include <boost/array.hpp>
#include <boost/bind.hpp>

#include "atlas/jsonrpc/result.hpp"
#include "atlas/jsonrpc/request.hpp"
#include "styx/serialise_json.hpp"

aether::radio_server::radio_server(
        boost::shared_ptr<boost::asio::io_service> io
        ) :
    atlas::api::server(io),
    m_io(io),
    m_port(*io, "/dev/ttyAMA0")
{
}

void aether::radio_server::start()
{
    async_read();
}

void aether::radio_server::async_read()
{
    boost::asio::async_read_until(
            m_port,
            m_buffer,
            "\n",
            boost::bind(
                &radio_server::async_read_received,
                shared_from_this(),
                boost::asio::placeholders::error
                )
            );
}

void aether::radio_server::async_read_received(
        const boost::system::error_code& ec
        )
{
    std::string line;
    {
        std::istream s(&m_buffer);
        std::getline(s, line);
    }
    //atlas::log::test("aether::radio_server::async_read_received") <<
        //"received " << line;
    try
    {
        styx::element e(styx::parse_json(line));
        atlas::jsonrpc::request request(e);
        auto this_ptr = shared_from_this();
        serve(
            request,
            [this_ptr](const atlas::jsonrpc::result& result) {
                //atlas::log::information("aether::radio_server::async_read_received") <<
                    //"result: " << styx::serialise_json(result);
                this_ptr->send_result(result);
            }
            );
    }
    catch(const std::exception& e)
    {
        atlas::log::error("aether::radio_server::async_read_received") <<
            "parsing \"" << line << "\" " << e.what();
        async_read();
    }
}

void aether::radio_server::send_result(const atlas::jsonrpc::result& result)
{
    const std::string out(styx::serialise_json(result));
    static const char eol[] = "\n";
    boost::array<boost::asio::const_buffer, 2> buffers{
        boost::asio::buffer(out), boost::asio::buffer(eol)
    };
    atlas::log::information("aether::radio_server::send_result") <<
        "sending " << out;
    boost::asio::async_write(
            m_port,
            buffers,
            boost::bind(
                &radio_server::send_result_complete,
                shared_from_this(),
                boost::asio::placeholders::error
                )
            );
}

void aether::radio_server::send_result_complete(
        const boost::system::error_code& ec
        )
{
    async_read();
}


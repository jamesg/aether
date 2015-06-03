#ifndef AETHER_SENSOR_API_HPP
#define AETHER_SENSOR_API_HPP


#include "atlas/api/server.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    class sensor_api :
        public virtual atlas::api::server
    {
    public:
        sensor_api(hades::connection& conn);
        sensor_api(
                boost::shared_ptr<boost::asio::io_service>,
                hades::connection& conn
                );
    private:
        void install_sensor_api(hades::connection& conn);
    };
}

#endif


#ifndef AETHER_TASK_RETRIEVE_FORECAST_HPP
#define AETHER_TASK_RETRIEVE_FORECAST_HPP

#include <boost/enable_shared_from_this.hpp>

#include "atlas/task/base.hpp"

namespace aether
{
    namespace openweathermap
    {
        class forecast;
    }
    namespace task
    {
        /*!
         * \brief Retrieve and store the latest weather forecase data from the remote API.
         *
         * \todo Currently really slow (writing the data to the database takes
         * about 30 seconds).  Make it faster or split it into multiple
         * callbacks.
         */
        class retrieve_forecast :
            public boost::enable_shared_from_this<retrieve_forecast>,
            public atlas::task::base,
            public atlas::task::has_connection
        {
        public:
            static boost::shared_ptr<retrieve_forecast> create(
                atlas::task::io_service_type io_,
                atlas::task::callback_function_type callback,
                hades::connection& conn
                );

            void run() override;
        private:
            retrieve_forecast(
                atlas::task::io_service_type io_,
                atlas::task::callback_function_type callback,
                hades::connection& conn
                );
            void forecast_received(openweathermap::forecast&);
            void forecast_error(const std::string&);
        };
    }
}

#endif


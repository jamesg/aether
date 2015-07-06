#ifndef AETHER_TEMPERATURE_MODEL_HPP
#define AETHER_TEMPERATURE_MODEL_HPP

#include <Eigen/Core>

#include "styx/object.hpp"

namespace hades
{
    class connection;
}

namespace aether
{
    class temperature_model
    {
    public:
        typedef float feature_type;

        explicit temperature_model(hades::connection& conn);

        /*!
        \brief Estimate the actual air temperature given the current weather
        forecast.
        */
        feature_type estimate(styx::object);

    private:
        typedef Eigen::Matrix<feature_type, Eigen::Dynamic, 1>
            feature_vector_type;

        /*!
        \brief Construct a column feature vector from a weather forecast.
        */
        feature_vector_type feature_vector(styx::object);

        /*
        \brief Learnt linear regression parameters.
        */
        feature_vector_type m_theta;
    };
}

#endif

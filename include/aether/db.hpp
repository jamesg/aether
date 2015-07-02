#ifndef AETHER_DB_HPP
#define AETHER_DB_HPP

#include "atlas/db/date_series.hpp"
#include "atlas/db/temporal.hpp"
#include "hades/crud.hpp"
#include "hades/has_candidate_key.hpp"
#include "hades/has_flags.hpp"
#include "hades/relation.hpp"
#include "hades/tuple.hpp"
#include "styx/atom.hpp"

namespace aether
{
    namespace attr
    {
        extern const char kb_family_id[];
        extern const char kb_family_cname[];
        extern const char kb_family_lname[];
        extern const char kb_family_desc[];

        extern const char kb_variety_id[];
        extern const char kb_variety_cname[];
        extern const char kb_variety_lname[];
        extern const char kb_variety_weeks[];
        extern const char kb_variety_colour[];
        // kb_family_id

        extern const char kb_variety_harvest_mon_month[];
        extern const char kb_variety_plant_mon_month[];
        extern const char kb_variety_sow_mon_month[];
        // kb_variety_id

        extern const char batch_id[];
        // kb_variety_id

        extern const char phase_id[];
        extern const char phase_desc[];

        // batch_id
        // phase_id
        extern const char start[];
        extern const char finish[];

        // phase_id
        extern const char phase_order[];

        // batch_id
        // phase_id

        extern const char sensor_id[];
        extern const char sensor_desc[];

        extern const char log_time[];
        extern const char moisture[];
        extern const char temperature[];

        extern const char location_city[];
        extern const char location_lat[];
        extern const char location_lon[];

        extern const char setting_name[];
        extern const char setting_value[];

        // Forecast points are keyed by the data receiving time (dt).  This is
        // a Unix time.
        extern const char forecast_dt[];

        extern const char forecast_clouds_all[];

        extern const char forecast_main_temp[];
        extern const char forecast_main_temp_min[];
        extern const char forecast_main_temp_max[];
        extern const char forecast_main_humidity[];
        extern const char forecast_main_pressure[];
        extern const char forecast_main_sea_level[];
        extern const char forecast_main_grnd_level[];

        extern const char forecast_rain[];

        extern const char forecast_weather_main[];
        extern const char forecast_weather_description[];

        extern const char forecast_wind_speed[];
        extern const char forecast_wind_deg[];
        extern const char forecast_wind_gust[];

        extern const char forecast_temp_day[];
        extern const char forecast_temp_night[];
    }
    namespace relvar
    {
        extern const char kb_family[];
        extern const char kb_variety[];
        extern const char kb_variety_harvest_mon[];
        extern const char kb_variety_plant_mon[];
        extern const char kb_variety_sow_mon[];

        extern const char batch[];
        extern const char batch_phase[];
        extern const char batch_phase_history[];
        extern const char phase[];
        extern const char phase_order[];

        extern const char temperature_log[];
        extern const char moisture_log[];
        extern const char sensor_at_batch[];
        extern const char sensor[];

        extern const char location[];

        extern const char setting[];

        extern const char forecast[];
        extern const char forecast_clouds[];
        extern const char forecast_main[];
        extern const char forecast_rain[];
        extern const char forecast_weather[];
        extern const char forecast_wind[];

        extern const char daily_forecast[];
    }
    namespace flag
    {
        extern const char kb_variety_container[];
        extern const char kb_variety_flower[];
        extern const char kb_variety_prefer_shade[];
        extern const char kb_variety_prefer_sun[];

        extern const char favourite_variety[];
        extern const char moisture_sensor[];
        extern const char temperature_sensor[];
    }
    namespace db
    {
        void create(hades::connection&);
    }

    namespace kb
    {
        class family :
            public hades::tuple<
                attr::kb_family_id,
                attr::kb_family_cname,
                attr::kb_family_lname,
                attr::kb_family_desc>,
            public hades::has_candidate_key<attr::kb_family_id>,
            public hades::relation<relvar::kb_family>,
            public hades::crud<family>
        {
        public:
            family()
            {
            }
            family(const styx::element& e) :
                styx::object(e)
            {
            }
        };
        class variety :
            public hades::tuple<
                attr::kb_variety_id,
                attr::kb_variety_cname,
                attr::kb_variety_lname,
                attr::kb_family_id,
                attr::kb_variety_weeks,
                attr::kb_variety_colour>,
            public hades::has_candidate_key<attr::kb_variety_id>,
            public hades::has_flags<
                flag::kb_variety_container,
                flag::kb_variety_flower,
                flag::kb_variety_prefer_shade,
                flag::kb_variety_prefer_sun>,
            public hades::relation<relvar::kb_variety>,
            public hades::crud<variety>
        {
        public:
            variety()
            {
            }
            variety(const styx::element& e) :
                styx::object(e)
            {
            }
        };
        class variety_harvest_mon :
            public hades::tuple<
                attr::kb_variety_id,
                attr::kb_variety_harvest_mon_month>,
            public hades::has_candidate_key<
                attr::kb_variety_id,
                attr::kb_variety_harvest_mon_month>,
            public hades::relation<relvar::kb_variety_harvest_mon>,
            public hades::crud<variety_harvest_mon>
        {
        public:
            variety_harvest_mon()
            {
            }
            variety_harvest_mon(const styx::element& e) :
                styx::object(e)
            {
            }
        };
        class variety_plant_mon :
            public hades::tuple<
                attr::kb_variety_id,
                attr::kb_variety_plant_mon_month>,
            public hades::has_candidate_key<
                attr::kb_variety_id,
                attr::kb_variety_plant_mon_month>,
            public hades::relation<relvar::kb_variety_plant_mon>,
            public hades::crud<variety_plant_mon>
        {
        public:
            variety_plant_mon()
            {
            }
            variety_plant_mon(const styx::element& e) :
                styx::object(e)
            {
            }
        };
        class variety_sow_mon :
            public hades::tuple<
                attr::kb_variety_id,
                attr::kb_variety_sow_mon_month>,
            public hades::has_candidate_key<
                attr::kb_variety_id,
                attr::kb_variety_sow_mon_month>,
            public hades::relation<relvar::kb_variety_sow_mon>,
            public hades::crud<variety_sow_mon>
        {
        public:
            variety_sow_mon()
            {
            }
            variety_sow_mon(const styx::element& e) :
                styx::object(e)
            {
            }
        };
    }
    class batch :
        public hades::tuple<attr::batch_id, attr::kb_variety_id>,
        public hades::has_candidate_key<attr::batch_id>,
        public hades::relation<relvar::batch>,
        public hades::crud<batch>
    {
    public:
        batch()
        {
        }
        batch(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class phase :
        public hades::tuple<attr::phase_id, attr::phase_desc>,
        public hades::has_candidate_key<attr::phase_id>,
        public hades::relation<relvar::phase>,
        public hades::crud<phase>
    {
    public:
        phase()
        {
        }
        phase(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class phase_order :
        public hades::tuple<attr::phase_id, attr::phase_order>,
        public hades::has_candidate_key<attr::phase_id>,
        public hades::relation<relvar::phase_order>,
        public hades::crud<phase_order>
    {
    public:
        phase_order()
        {
        }
        phase_order(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class batch_phase :
        public hades::tuple<attr::batch_id, attr::phase_id, attr::start>,
        public hades::has_candidate_key<attr::batch_id>,
        public hades::relation<relvar::batch_phase>,
        public hades::crud<batch_phase>
    {
    public:
        batch_phase()
        {
        }
        batch_phase(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class batch_phase_history :
        public hades::tuple<
            attr::batch_id,
            attr::phase_id,
            attr::start,
            attr::finish>,
        public hades::has_candidate_key<attr::batch_id, attr::phase_id>,
        public hades::relation<relvar::batch_phase_history>,
        public hades::crud<batch_phase_history>
    {
    public:
        batch_phase_history()
        {
        }
        batch_phase_history(const styx::element& e) :
            styx::object(e)
        {
        }
        /*!
         * \brief Add the finish date attribute using the current time.
         */
        batch_phase_history(const batch_phase&);
    };
    class sensor :
        public hades::tuple<attr::sensor_id, attr::sensor_desc>,
        public hades::has_candidate_key<attr::sensor_id>,
        public hades::has_flags<
            flag::moisture_sensor,
            flag::temperature_sensor>,
        public hades::relation<relvar::sensor>,
        public hades::crud<sensor>
    {
    public:
        sensor()
        {
        }
        sensor(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class sensor_at_batch :
        public hades::tuple<attr::sensor_id, attr::batch_id>,
        public hades::has_candidate_key<attr::batch_id>,
        public hades::relation<relvar::sensor_at_batch>,
        public hades::crud<sensor_at_batch>
    {
    };

    //
    // Sensor logs.
    //

    typedef atlas::db::date_series<
        batch::id_type,
        relvar::moisture_log,
        attr::moisture,
        attr::log_time>
        moisture_log;
    typedef atlas::db::date_series<
        batch::id_type,
        relvar::temperature_log,
        attr::temperature,
        attr::log_time>
        temperature_log;
    class location :
        public hades::tuple<
            attr::location_city,
            attr::location_lat,
            attr::location_lon>,
        public hades::has_candidate_key<>,
        public hades::relation<relvar::location>,
        public hades::crud<location>
    {
    public:
        location()
        {
        }
        location(const styx::element& e) :
            styx::object(e)
        {
        }
    };
    class setting :
        public hades::tuple<attr::setting_name, attr::setting_value>,
        public hades::has_candidate_key<attr::setting_name>,
        public hades::relation<relvar::setting>,
        public hades::crud<setting>
    {
    public:
        setting()
        {
        }
        setting(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    //
    // Weather Forecast.
    //

    /*!
     * \brief Basic forecast; other forecast tables reference this table.
     */
    class forecast :
        public hades::tuple<attr::forecast_dt>,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast>,
        public hades::crud<forecast>
    {
    };

    class forecast_clouds :
        public hades::tuple<attr::forecast_dt, attr::forecast_clouds_all>,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast_clouds>,
        public hades::crud<forecast_clouds>
    {
    };

    class forecast_main :
        public hades::tuple<
            attr::forecast_dt,
            attr::forecast_main_temp,
            attr::forecast_main_temp_min,
            attr::forecast_main_temp_max,
            attr::forecast_main_humidity,
            attr::forecast_main_pressure,
            attr::forecast_main_sea_level,
            attr::forecast_main_grnd_level
            >,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast_main>,
        public hades::crud<forecast_main>
    {
    };

    class forecast_rain :
        public hades::tuple<attr::forecast_dt, attr::forecast_rain>,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast_rain>,
        public hades::crud<forecast_rain>
    {
    };

    class forecast_weather :
        public hades::tuple<
            attr::forecast_dt,
            attr::forecast_weather_main,
            attr::forecast_weather_description
            >,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast_weather>,
        public hades::crud<forecast_weather>
    {
    };

    class forecast_wind :
        public hades::tuple<
            attr::forecast_dt,
            attr::forecast_wind_speed,
            attr::forecast_wind_deg,
            attr::forecast_wind_gust
            >,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::forecast_wind>,
        public hades::crud<forecast_wind>
    {
    };

    class daily_forecast :
        public hades::tuple<
            attr::forecast_dt,
            attr::forecast_weather_main,
            attr::forecast_weather_description,
            attr::forecast_temp_day,
            attr::forecast_temp_night,
            attr::forecast_wind_speed,
            attr::forecast_wind_deg,
            attr::forecast_clouds_all,
            attr::forecast_rain
            >,
        public hades::has_candidate_key<attr::forecast_dt>,
        public hades::relation<relvar::daily_forecast>,
        public hades::crud<daily_forecast>
    {
    };

    namespace db
    {
        /*!
         * \brief Get all settings as key -> atom pairs.
         */
        styx::object settings(hades::connection&);
        /*!
         * \brief Save an object of key -> atom pairs as settings.
         */
        void save_settings(styx::object settings, hades::connection&);

        /*!
         * \brief Get the value of a boolean setting.
         * \throw std::exception if the setting is not found or cannot be
         * interpreted as a boolean.
         */
        bool bool_setting(
            hades::connection&,
            const std::string& key
        );
        /*!
         * \brief Get the value of a boolean setting, or return a default value
         * if the setting is unavailable.
         * \param default_value Value to return if the setting is not found or
         * cannot be interpreted as a boolean.
         */
        bool bool_setting(
            hades::connection&,
            const std::string& key,
            bool default_value
        );
        /*!
         * \brief Get the value of an integer setting.
         * \throw std::exception if the setting is not found or cannot be
         * interpreted as an integer.
         */
        int int_setting(
            hades::connection&,
            const std::string& key
        );
        /*!
         * \brief Get the value of an integer setting, or return a default
         * value if the setting is unavailable.
         * \param default_value Value to return if the setting is not found or
         * cannot be interpreted as an integer.
         */
        int int_setting(
            hades::connection&,
            const std::string& key,
            int default_value
        );
        /*!
         * \brief Get the value of a string setting.
         * \throw std::exception if the setting is not found or cannot be
         * interpreted as a string.
         */
        std::string string_setting(
            hades::connection&,
            const std::string& key
        );
        /*!
         * \brief Get the value of a string setting, or return a default value
         * if the setting is unavailable.
         * \param default_value Value to return if the setting is not found or
         * cannot be interpreted as a string.
         */
        std::string string_setting(
            hades::connection&,
            const std::string& key,
            const std::string& default_value
        );
    }
}

#endif

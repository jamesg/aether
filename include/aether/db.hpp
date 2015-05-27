#ifndef AETHER_DB_HPP
#define AETHER_DB_HPP

#include "atlas/db/temporal.hpp"
#include "hades/crud.hpp"
#include "hades/has_candidate_key.hpp"
#include "hades/has_flags.hpp"
#include "hades/relation.hpp"
#include "hades/tuple.hpp"

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
        // kb_family_id

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
    }
    namespace relvar
    {
        extern const char kb_family[];
        extern const char kb_variety[];
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
                attr::kb_variety_weeks>,
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
        class variety_sow_mon :
            public hades::tuple<
                attr::kb_variety_id,
                attr::kb_variety_sow_mon_month>,
            public hades::has_candidate_key<attr::kb_variety_id>,
            public hades::relation<relvar::kb_variety_sow_mon>,
            public hades::crud<variety_sow_mon>
        {
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
        public hades::has_candidate_key<attr::sensor_id>,
        public hades::relation<relvar::sensor_at_batch>,
        public hades::crud<sensor_at_batch>
    {
    };
    class moisture_log :
        public atlas::db::semi_temporal<sensor::id_type, relvar::moisture_log>
    {
    };
    class temperature_log :
        public atlas::db::semi_temporal<sensor::id_type, relvar::temperature_log>
    {
    };
}

#endif


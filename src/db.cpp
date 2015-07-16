#include "aether/db.hpp"

#include "atlas/db/date.hpp"
#include "hades/crud.ipp"
#include "hades/custom_select_one.hpp"
#include "hades/devoid.hpp"
#include "hades/exists.hpp"

namespace
{
    template<typename Atom>
    Atom setting_value(
            hades::connection& conn,
            const std::string& setting_name
    )
    {
        return (
            hades::get_one<aether::setting>(
                conn,
                hades::where(
                    "setting_name = ? ",
                    hades::row<std::string>(setting_name)
                )
            )
        ).get_attr<aether::attr::setting_value, Atom>();
    }
    template<typename Atom>
    Atom setting_value(
            hades::connection& conn,
            const std::string& setting_name,
            const Atom default_value
    )
    {
        try
        {
            if(
                hades::exists<aether::setting>(
                    conn,
                    hades::where(
                        "setting_name = ?",
                        hades::row<std::string>(setting_name)
                    )
                )
              )
                return hades::get_one<aether::setting>(
                        conn,
                        hades::where(
                            "setting_name = ?",
                            hades::row<std::string>(setting_name)
                        )
                    ).get_attr<aether::attr::setting_value, Atom>();
        }
        catch(const std::exception& e)
        {
            atlas::log::warning("aether::db setting_value") <<
                "getting setting value: " << e.what();
        }
        return default_value;
    }
}

const char aether::attr::kb_family_id[] = "kb_family_id";
const char aether::attr::kb_family_cname[] = "kb_family_cname";
const char aether::attr::kb_family_lname[] = "kb_family_lname";
const char aether::attr::kb_family_desc[] = "kb_family_desc";
const char aether::attr::kb_variety_id[] = "kb_variety_id";
const char aether::attr::kb_variety_cname[] = "kb_variety_cname";
const char aether::attr::kb_variety_lname[] = "kb_variety_lname";
const char aether::attr::kb_variety_weeks[] = "kb_variety_weeks";
const char aether::attr::kb_variety_colour[] = "kb_variety_colour";
const char aether::attr::kb_variety_harvest_mon_month[] = "kb_variety_harvest_mon_month";
const char aether::attr::kb_variety_plant_mon_month[] = "kb_variety_plant_mon_month";
const char aether::attr::kb_variety_sow_mon_month[] = "kb_variety_sow_mon_month";
const char aether::attr::batch_id[] = "batch_id";
const char aether::attr::phase_id[] = "phase_id";
const char aether::attr::phase_desc[] = "phase_desc";
const char aether::attr::start[] = "start";
const char aether::attr::finish[] = "finish";
const char aether::attr::phase_order[] = "phase_order";
const char aether::attr::sensor_id[] = "sensor_id";
const char aether::attr::sensor_desc[] = "sensor_desc";
const char aether::attr::log_time[] = "log_time";
const char aether::attr::moisture[] = "moisture";
const char aether::attr::temperature[] = "temperature";
const char aether::attr::location_city[] = "location_city";
const char aether::attr::location_lat[] = "location_lat";
const char aether::attr::location_lon[] = "location_lon";
const char aether::attr::setting_name[] = "setting_name";
const char aether::attr::setting_value[] = "setting_value";
const char aether::attr::forecast_dt[] = "forecast_dt";
const char aether::attr::forecast_clouds_all[] = "forecast_clouds_all";
const char aether::attr::forecast_main_temp[] = "forecast_main_temp";
const char aether::attr::forecast_main_temp_min[] = "forecast_main_temp_min";
const char aether::attr::forecast_main_temp_max[] = "forecast_main_temp_max";
const char aether::attr::forecast_main_humidity[] = "forecast_main_humidity";
const char aether::attr::forecast_main_pressure[] = "forecast_main_pressure";
const char aether::attr::forecast_main_sea_level[] = "forecast_main_sea_level";
const char aether::attr::forecast_main_grnd_level[] = "forecast_main_grnd_level";
const char aether::attr::forecast_rain[] = "forecast_rain";
const char aether::attr::forecast_weather_main[] = "forecast_weather_main";
const char aether::attr::forecast_weather_description[] = "forecast_weather_description";
const char aether::attr::forecast_wind_speed[] = "forecast_wind_speed";
const char aether::attr::forecast_wind_deg[] = "forecast_wind_deg";
const char aether::attr::forecast_wind_gust[] = "forecast_wind_gust";
const char aether::attr::forecast_temp_day[] = "forecast_temp_day";
const char aether::attr::forecast_temp_night[] = "forecast_temp_night";
const char aether::relvar::kb_family[] = "aether_kb_family";
const char aether::relvar::kb_variety[] = "aether_kb_variety";
const char aether::relvar::kb_variety_harvest_mon[] = "aether_kb_variety_harvest_mon";
const char aether::relvar::kb_variety_plant_mon[] = "aether_kb_variety_plant_mon";
const char aether::relvar::kb_variety_sow_mon[] = "aether_kb_variety_sow_mon";
const char aether::relvar::batch[] = "aether_batch";
const char aether::relvar::batch_phase[] = "aether_batch_phase";
const char aether::relvar::batch_phase_history[] = "aether_batch_phase_history";
const char aether::relvar::phase[] = "aether_phase";
const char aether::relvar::phase_order[] = "aether_phase_order";
const char aether::relvar::temperature_log[] = "aether_temperature_log";
const char aether::relvar::moisture_log[] = "aether_moisture_log";
const char aether::relvar::sensor_at_batch[] = "aether_sensor_at_batch";
const char aether::relvar::sensor[] = "aether_sensor";
const char aether::relvar::location[] = "aether_location";
const char aether::relvar::setting[] = "aether_setting";
const char aether::relvar::forecast[] = "aether_forecast";
const char aether::relvar::forecast_clouds[] = "aether_forecast_clouds";
const char aether::relvar::forecast_rain[] = "aether_forecast_rain";
const char aether::relvar::forecast_main[] = "aether_forecast_main";
const char aether::relvar::forecast_weather[] = "aether_forecast_weather";
const char aether::relvar::forecast_wind[] = "aether_forecast_wind";
const char aether::relvar::daily_forecast[] = "aether_daily_forecast";
const char aether::view::sensor_at_phase[] = "aether_sensor_at_phase";
const char aether::view::phase_temperature[] = "aether_phase_temperature";
const char aether::flag::kb_variety_container[] = "aether_kb_variety_container";
const char aether::flag::kb_variety_flower[] = "aether_kb_variety_flower";
const char aether::flag::kb_variety_prefer_shade[] = "aether_kb_variety_prefer_shade";
const char aether::flag::kb_variety_prefer_sun[] = "aether_kb_variety_prefer_sun";
const char aether::flag::favourite_variety[] = "aether_favourite_variety";
const char aether::flag::moisture_sensor[] = "aether_moisture_sensor";
const char aether::flag::temperature_sensor[] = "aether_temperature_sensor";

aether::batch_phase_history::batch_phase_history(const batch_phase& e) :
    styx::object(e)
{
    get_int<attr::finish>() = atlas::db::date::to_unix_time(
            boost::posix_time::second_clock::universal_time()
            );
}

void aether::db::create(hades::connection& conn)
{
    //
    // Knowledge base types.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_family ( "
        " kb_family_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " kb_family_cname VARCHAR, "
        " kb_family_lname VARCHAR, "
        " kb_family_desc VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety ( "
        " kb_variety_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " kb_variety_cname VARCHAR, "
        " kb_variety_lname VARCHAR, "
        " kb_variety_weeks INTEGER, "
        " kb_variety_colour VARCHAR, "
        " kb_family_id INTEGER REFERENCES aether_kb_family(kb_family_id) "
        " ) ",
        conn
        );

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_harvest_mon ( "
        " kb_variety_id INTEGER, "
        " kb_variety_harvest_mon_month INTEGER, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE, "
        " UNIQUE(kb_variety_id, kb_variety_harvest_mon_month) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_plant_mon ( "
        " kb_variety_id INTEGER, "
        " kb_variety_plant_mon_month INTEGER, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE, "
        " UNIQUE(kb_variety_id, kb_variety_plant_mon_month) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_sow_mon ( "
        " kb_variety_id INTEGER, "
        " kb_variety_sow_mon_month INTEGER, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE, "
        " UNIQUE(kb_variety_id, kb_variety_sow_mon_month) "
        " ) ",
        conn
        );

    //
    // Knowledge base variety flags.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_container ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_flower ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_prefer_shade ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_prefer_sun ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) ON DELETE CASCADE "
        " ) ",
        conn
        );


    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch ( "
        " batch_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " kb_variety_id INTEGER REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );

    //
    // Phase history.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_phase ( "
        " phase_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " phase_desc VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_phase_order ( "
        " phase_id INTEGER PRIMARY KEY, "
        " phase_order INTEGER, "
        " FOREIGN KEY(phase_id) REFERENCES aether_phase(phase_id) ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch_phase ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " phase_id INTEGER REFERENCES aether_phase(phase_id), "
        " start INTEGER, "
        " UNIQUE(batch_id, phase_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch_phase_history ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " phase_id INTEGER REFERENCES aether_phase(phase_id), "
        " start INTEGER, "
        " finish INTEGER "
        " ) ",
        conn
        );

    //
    // Sensor.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_sensor ( "
        " sensor_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " sensor_desc VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_sensor_at_batch ( "
        " batch_id INTEGER PRIMARY KEY, "
        " sensor_id INTEGER, "
        " FOREIGN KEY(batch_id) REFERENCES aether_batch(batch_id), "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) "
        "  ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE VIEW IF NOT EXISTS aether_sensor_at_phase AS "
        " SELECT DISTINCT "
        "  aether_sensor_at_batch.sensor_id, aether_batch_phase.phase_id "
        " FROM aether_sensor_at_batch "
        " JOIN aether_batch_phase "
        " ON aether_sensor_at_batch.batch_id = aether_batch_phase.batch_id ",
        conn
    );

    //
    // Sensor logs.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_moisture_log ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " log_time INTEGER, "
        " moisture INTEGER, "
        " PRIMARY KEY(batch_id, log_time) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_temperature_log ( "
        " phase_id INTEGER REFERENCES aether_phase(phase_id), "
        " log_time INTEGER, "
        " temperature REAL, "
        " PRIMARY KEY(phase_id, log_time) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE VIEW IF NOT EXISTS aether_phase_temperature AS "
        "  SELECT "
        "   phase_id, temperature, MAX(log_time) AS log_time "
        "  FROM ( "
        "   SELECT "
        "    aether_phase.phase_id AS phase_id, "
        "    strftime('%s', 'now') AS current_time, "
        "    aether_temperature_log.temperature AS temperature, "
        "    aether_temperature_log.log_time AS log_time, "
        "    strftime('%s', 'now') - aether_temperature_log.log_time as time_diff "
        "   FROM aether_temperature_log "
        "   JOIN aether_phase "
        "   ON aether_temperature_log.phase_id = aether_phase.phase_id "
        "   WHERE time_diff >= 0 AND time_diff < 10800"
        "  ) "
        "  GROUP BY phase_id ",
        conn
    );

    //
    // Sensor flags.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_moisture_sensor ( "
        " sensor_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_temperature_sensor ( "
        " sensor_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) ON DELETE CASCADE "
        " ) ",
        conn
        );

    //
    // Location singleton.
    //
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_location ( "
        " location_city VARCHAR, "
        " location_lat REAL, "
        " location_lon REAL "
        " ) ",
        conn
        );

    //
    // Settings
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_setting ( "
        " setting_name VARCHAR, "
        " setting_value VARCHAR, "
        " UNIQUE(setting_name) "
        " ) ",
        conn
        );

    //
    // Weather Forecast.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast ( "
        " forecast_dt INTEGER PRIMARY KEY "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast_clouds ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_clouds_all INTEGER "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast_main ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_main_temp REAL, "
        " forecast_main_temp_min REAL, "
        " forecast_main_temp_max REAL, "
        " forecast_main_humidity INTEGER, "
        " forecast_main_pressure REAL, "
        " forecast_main_sea_level REAL, "
        " forecast_main_grnd_level REAL "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast_rain ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_rain REAL "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast_weather ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_weather_main TEXT, "
        " forecast_weather_description TEXT "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_forecast_wind ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_wind_speed REAL, "
        " forecast_wind_deg REAL, "
        " forecast_wind_gust REAL "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_daily_forecast ( "
        " forecast_dt INTEGER PRIMARY KEY, "
        " forecast_temp_day REAL, "
        " forecast_temp_night REAL, "
        " forecast_weather_main TEXT, "
        " forecast_weather_description TEXT, "
        " forecast_wind_speed REAL, "
        " forecast_wind_deg REAL, "
        " forecast_clouds_all INTEGER, "
        " forecast_rain REAL "
        " ) ",
        conn
        );

    //
    // Initial data.
    //

    // Insert a default sensor.
    // TODO allow for more than one sensor.
    if(!hades::exists<sensor>(conn, hades::where("sensor_id = 0")))
    {
        sensor default_sensor;
        default_sensor.get_int<attr::sensor_id>() = 0;
        default_sensor.get_string<attr::sensor_desc>() = "Default Sensor";
        default_sensor.insert(conn);
    }
}

styx::object aether::db::settings(hades::connection& conn)
{
    styx::object out;
    styx::list db_settings(setting::get_collection(conn));
    for(const styx::element& e : db_settings)
    {
        setting s(e);
        out[s.get_string<attr::setting_name>()] = s[attr::setting_value];
    }
    return out;
}

void aether::db::save_settings(
        styx::object settings,
        hades::connection& conn
)
{
    styx::list l;
    for(std::pair<std::string, styx::element> p : settings)
    {
        setting s;
        s.get_string<aether::attr::setting_name>() = p.first;
        s.get_element(aether::attr::setting_value) = p.second;
        l.append(s);
    }
    setting::save_collection(l, conn);
}

bool aether::db::bool_setting(
        hades::connection& conn,
        const std::string& setting_name
        )
{
    return setting_value<bool>(conn, setting_name);
}

bool aether::db::bool_setting(
        hades::connection& conn,
        const std::string& setting_name,
        const bool default_value
        )
{
    return setting_value<bool>(conn, setting_name, default_value);
}

styx::int_type aether::db::int_setting(
        hades::connection& conn,
        const std::string& setting_name
        )
{
    return setting_value<styx::int_type>(conn, setting_name);
}

styx::int_type aether::db::int_setting(
        hades::connection& conn,
        const std::string& setting_name,
        const styx::int_type default_value
        )
{
    return setting_value<styx::int_type>(conn, setting_name, default_value);
}

std::string aether::db::string_setting(
        hades::connection& conn,
        const std::string& setting_name
        )
{
    return setting_value<std::string>(conn, setting_name);
}

std::string aether::db::string_setting(
        hades::connection& conn,
        const std::string& setting_name,
        const std::string& default_value
        )
{
    return setting_value<std::string>(conn, setting_name, default_value);
}

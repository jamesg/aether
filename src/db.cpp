#include "aether/db.hpp"

#include "hades/crud.ipp"
#include "hades/devoid.hpp"

const char aether::attr::kb_family_id[] = "kb_family_id";
const char aether::attr::kb_family_cname[] = "kb_family_cname";
const char aether::attr::kb_family_lname[] = "kb_family_lname";
const char aether::attr::kb_family_desc[] = "kb_family_desc";
const char aether::attr::kb_variety_id[] = "kb_variety_id";
const char aether::attr::kb_variety_cname[] = "kb_variety_cname";
const char aether::attr::kb_variety_lname[] = "kb_variety_lname";
const char aether::attr::kb_variety_weeks[] = "kb_variety_weeks";
const char aether::attr::kb_variety_sow_mon_month[] = "kb_variety_sow_mon_month";
const char aether::attr::batch_id[] = "batch_id";
const char aether::attr::phase_id[] = "phase_id";
const char aether::attr::phase_desc[] = "phase_desc";
const char aether::attr::start[] = "start";
const char aether::attr::finish[] = "finish";
const char aether::attr::phase_order[] = "phase_order";
const char aether::attr::sensor_id[] = "sensor_id";
const char aether::attr::sensor_desc[] = "sensor_desc";
const char aether::relvar::kb_family[] = "aether_kb_family";
const char aether::relvar::kb_variety[] = "aether_kb_variety";
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
const char aether::flag::kb_variety_container[] = "aether_kb_variety_container";
const char aether::flag::kb_variety_flower[] = "aether_kb_variety_flower";
const char aether::flag::kb_variety_prefer_shade[] = "aether_kb_variety_prefer_shade";
const char aether::flag::kb_variety_prefer_sun[] = "aether_kb_variety_prefer_sun";
const char aether::flag::favourite_variety[] = "aether_favourite_variety";
const char aether::flag::moisture_sensor[] = "aether_moisture_sensor";
const char aether::flag::temperature_sensor[] = "aether_temperature_sensor";

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
        " kb_family_id INTEGER REFERENCES aether_kb_family(kb_family_id) "
        " ) ",
        conn
        );

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_sow_mon ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " kb_variety_sow_mon_month INTEGER, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );

    //
    // Knowledge base variety flags.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_container ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_flower ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_prefer_shade ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_kb_variety_prefer_sun ( "
        " kb_variety_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(kb_variety_id) REFERENCES aether_kb_variety(kb_variety_id) "
        " ) ",
        conn
        );


    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch ( "
        " batch_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " kb_variety_id INTEGER REFERENCES kb_variety(kb_variety_id) "
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
        " FOREIGN KEY(phase_id) REFERENCES aether_phase(phase_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch_phase ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " phase_id INTEGER REFERENCES aether_phase(phase_id), "
        " start VARCHAR, "
        " UNIQUE(batch_id, phase_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_batch_phase_history ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " phase_id INTEGER REFERENCES aether_phase(phase_id), "
        " start VARCHAR, "
        " finish VARCHAR "
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
        " sensor_id INTEGER PRIMARY KEY, "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) "
        " ) ",
        conn
        );

    //
    // Sensor logs.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS moisture_log ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " date VARCHAR, "
        " moisture INTEGER, "
        " PRIMARY KEY(batch_id, date) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS temperature_log ( "
        " batch_id INTEGER REFERENCES aether_batch(batch_id), "
        " date VARCHAR, "
        " temperature REAL, "
        " PRIMARY KEY(batch_id, date) "
        " ) ",
        conn
        );

    //
    // Sensor flags.
    //

    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_moisture_sensor ( "
        " sensor_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS aether_temperature_sensor ( "
        " sensor_id INTEGER PRIMARY KEY, "
        " FOREIGN KEY(sensor_id) REFERENCES aether_sensor(sensor_id) "
        " ) ",
        conn
        );
}


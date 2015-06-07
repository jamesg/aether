#ifndef AETHER_SENSOR_TIMER_HPP
#define AETHER_SENSOR_TIMER_HPP

#include "Arduino.h"

class timer
{
public:
    typedef int id_type;
    typedef void(*callback_type)(void);

    /*!
     * \brief The maximum number of timers supported.
     */
    static const size_t ENTRY_COUNT = 8;

    /*!
     * \brief Id to use for an invalid timer.
     */
    static const int INVALID_ID = -1;

    timer();

    id_type after(long millis, callback_type callback);
    /*!
     * \brief Stop all timers from firing.
     */
    void reset();
    void stop(id_type timer_id);

    void update();

private:
    struct entry
    {
        entry();

        callback_type callback;
        long time;
    };

    int next_timer() const;

    entry m_entries[ENTRY_COUNT];
};

#endif


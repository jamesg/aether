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

    /*!
     * \brief Add a timer to fire once after 'millis' milliseconds.
     *
     * \param millis Number of milliseconds to wait (minimum) before the timer is fired.
     * \param callback Nullary function to call.
     *
     * \return The unique (among current timers) id of the timer.
     */
    id_type after(long millis, callback_type callback);

    /*!
     * \brief Stop all timers from firing.
     */
    void reset();

    /*!
     * \brief Stop a single timer.
     *
     * \param timer_id Id of the timer to stop.
     */
    void stop(id_type timer_id);

    /*!
     * \brief Fire any timers that have expired since update() was last called.
     */
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


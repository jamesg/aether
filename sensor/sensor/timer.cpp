#include "timer.hpp"

timer::id_type timer::after(long millis_, callback_type callback)
{
    int timer_id = next_timer();
    if(timer_id == INVALID_ID)
        return INVALID_ID;
    entry *next = &(m_entries[timer_id]);
    next->time = millis() + millis_;
    next->callback = callback;
}

void timer::reset()
{
    for(int i = 0; i < ENTRY_COUNT; ++i)
        m_entries[i] = entry();
}

void timer::stop(int timer_id)
{
    if(timer_id >= 0 && timer_id < ENTRY_COUNT)
        m_entries[timer_id] = entry();
}

void timer::update()
{
    for(int i = 0; i < ENTRY_COUNT; ++i)
    {
        if(m_entries[i].callback && millis() > m_entries[i].time)
        {
            callback_type cb = m_entries[i].callback;
            m_entries[i] = entry();
            cb();
        }
    }
}

int timer::next_timer() const
{
    for(int i = 0; i < ENTRY_COUNT; ++i)
        if(m_entries[i].callback == 0)
            return i;
    return INVALID_ID;
}

timer::entry::entry() :
    callback(0),
    time(0)
{
}

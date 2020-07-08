#pragma once
#ifndef INCLUDE_TAS_SIMPLE_TIMER_H
#define INCLUDE_TAS_SIMPLE_TIMER_H

#include "types.h"

class tas_simple_timer
{
public:
    tas_simple_timer(tas_size _timeout) noexcept:
        m_timeout(_timeout),
        m_timer(_timeout)
    {}


    void reset() noexcept
    {
        m_timer = m_timeout;
    }


    void reset(tas_size _new_timeout) noexcept
    {
        m_timer = _new_timeout;
        m_timeout = _new_timeout;
    }


    bool update(tas_size _delta) noexcept
    {
        if (m_timer <= _delta)
        {
            m_timer = m_timeout;
            return true;
        }
        m_timer -= _delta;
        return false;
    }
private:
    tas_size m_timeout;
    tas_size m_timer;
};

#endif //INCLUDE_TAS_SIMPLE_TIMER_H
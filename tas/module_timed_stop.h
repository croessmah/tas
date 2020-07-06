#pragma once
#ifndef INCLUDE_TAS_MODULE_TIMED_STOP_H
#define INCLUDE_TAS_MODULE_TIMED_STOP_H

#include "types.h"


struct tas_md_timed_stop
{
    tas_md_timed_stop(tas_size _timeout): m_start_time(0), m_timeout(_timeout) {}
    tas_error start(class tas_application & _app) noexcept;
    void stop(class tas_application & _app) noexcept;
    void tick(class tas_application & _app) noexcept;
private:
    tas_size m_start_time;
    tas_size m_timeout;
};


#endif //INCLUDE_TAS_MODULE_TIMED_STOP_H
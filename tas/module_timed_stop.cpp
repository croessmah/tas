#include <Windows.h>
#include "module_timed_stop.h"
#include "application.h"


tas_error tas_md_timed_stop::start(tas_application & _app) noexcept
{
    m_start_time = GetTickCount();
    return ERROR_SUCCESS;
}


void tas_md_timed_stop::stop(tas_application & _app) noexcept
{

}


void tas_md_timed_stop::tick(tas_application & _app) noexcept
{
    DWORD elapsed_time = GetTickCount() - m_start_time;
    if (elapsed_time > m_timeout)
    {
        _app.stop(100);
    }
}
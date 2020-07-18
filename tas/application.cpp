#include <Windows.h>
#include "application.h"
#include "pipe.h"

namespace
{
    wchar_t const * gc_stop_pipe_name = L"\\\\.\\pipe\\tas_stop_pipe";
}//internal linkage


tas_application::tas_application(tas_size _max_wait_timeout) :
    m_osrv(),
    m_max_timeout(_max_wait_timeout),
    m_stopped_event(NULL),
    m_running(false),
    m_stop_mech_ready(false),
    m_error(ERROR_SUCCESS)
{
    set_owned_thread();
    m_stopped_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_error = m_stop_pipe.create(gc_stop_pipe_name, { 1, 0, 0, eTAS_NPIPE_MODE::TAS_NP_MODE_INOUT });
    if (m_error)
    {
        return;
    }
    //todo: log trace
}


tas_application::~tas_application()
{
    if (m_stopped_event)
    {
        CloseHandle(m_stopped_event);
    }
    //check running and log
}


tas_error
tas_application::stop(tas_size _timeout) noexcept
{
    HANDLE pipe = INVALID_HANDLE_VALUE;
    pipe = CreateFileW(gc_stop_pipe_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }
    CloseHandle(pipe);
    if (is_owned_thread())
    {
        _timeout = 0;
    }

    DWORD ret = WaitForSingleObject(m_stopped_event, _timeout);
    if (ret == WAIT_OBJECT_0)
    {
        return ERROR_SUCCESS;
    }
    if (ret == WAIT_TIMEOUT)
    {
        return is_owned_thread() ? ERROR_SUCCESS : ERROR_TIMEOUT;
    }
    return GetLastError();
}


bool
tas_application::is_running() noexcept
{
    return m_running;
}


tas_overlapped_server &
tas_application::overlapped_server() noexcept
{
    return m_osrv;
}


tas_error
tas_application::error() noexcept
{
    return m_error;
}


tas_size 
tas_application::tick_time() noexcept
{
    return m_max_timeout;
}


tas_size tas_application::current_time() noexcept
{
    return GetTickCount();
}


bool
tas_application::init_run() noexcept
{
    m_error = ERROR_SUCCESS;
    restore_stop_event();
    if (!ready())
    {
        m_error = eTAS_APP_ERROR::TAS_AER_NOT_READY;
    }
    return m_error == ERROR_SUCCESS;
}


bool
tas_application::wait_next() noexcept
{
    tas_error err = m_osrv.wait(m_max_timeout);
    if (err == ERROR_SUCCESS || err == ERROR_TIMEOUT)
    {
        m_error = ERROR_SUCCESS;
        return true;
    }
    //todo: log trace
    m_error = err;
    return false;
}


void
tas_application::reset_stop_event() noexcept
{
    ResetEvent(m_stopped_event);
}


void
tas_application::set_stop_event() noexcept
{
    SetEvent(m_stopped_event);
}


void
tas_application::restore_stop_event() noexcept
{
    if (m_stop_mech_ready)
    {
        return;
    }
    m_error = ERROR_SUCCESS;
    if (!m_osrv.is_aborted())
    {
        m_error = m_osrv.connect_named_pipe(m_stop_pipe.handle(), nullptr, 0, stop_signal_receive, this);
        m_stop_mech_ready = (m_error == ERROR_SUCCESS);
    }
}


void
tas_application::stop_event() noexcept
{
    m_running = false;
    m_stop_mech_ready = false;
    m_stop_pipe.disconnect();
}


void
tas_application::set_owned_thread() noexcept
{
    m_owned_thread = GetCurrentThread();
}


bool
tas_application::is_owned_thread() noexcept
{
    return m_owned_thread == GetCurrentThread();
}


bool
tas_application::ready() noexcept
{
    //todo: check stopped and controll pipes
    return is_owned_thread() &&
        !m_running &&
        m_stop_mech_ready &&
        m_osrv.create_success() &&
        !m_osrv.is_aborted()
        ;
}


void
tas_application::stop_signal_receive(tas_overlapped_server &, tas_operation const & _operation, tas_error )
{
    tas_application * app = static_cast<tas_application *>(_operation.exdata);
    app->stop_event();
}

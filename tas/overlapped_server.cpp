//#include "tasos_aux.inl"
#include <Windows.h>
#include "overlapped_server.h"


tas_overlapped_server::tas_overlapped_server() noexcept:
    m_overlappeds_storage{},
    m_operations_storage{},
    m_operations{},
    m_overlappeds{},
    m_events{},
    m_events_count{},
    m_aborted(false)
{
    static_assert(sc_max_handles <= MAXIMUM_WAIT_OBJECTS, "Too many handles");
    for (tas_size i = 0; i < sc_max_handles; ++i)
    {
        m_operations[i] = m_operations_storage + i;
        m_overlappeds[i] = m_overlappeds_storage + i;
    }
}


tas_overlapped_server::~tas_overlapped_server() noexcept
{
    if (m_events_count != 0)
    {
        abort_and_wait_all();
    }
}


bool
tas_overlapped_server::create_success() noexcept
{
    for (auto & o : m_overlappeds)
    {
        if (!o->is_valid_event())
        {
            return false;
        }
    }
    return true;
}


tas_error
tas_overlapped_server::connect_named_pipe(
    tas_handle _handle, 
    char * _buf, 
    tas_size _size, 
    tas_operation_callback _callback, 
    void * _exdata) noexcept
{
    tas_size index = attach_operation(_handle, _buf, _size, _callback, _exdata);    
    if (index == sc_invalid_index)
    {
        return is_aborted()? ERROR_OPERATION_ABORTED: ERROR_IO_PENDING;
    }
    tas_error err = ERROR_SUCCESS;
    bool success = ConnectNamedPipe(_handle, overlapped(index)->raw());
    err = GetLastError();
    if (!success)
    {
        if (err == ERROR_IO_PENDING)
        {
            err = ERROR_SUCCESS;
        }
        else if (err == ERROR_PIPE_CONNECTED)
        {
            err = ERROR_SUCCESS;
            SetEvent(event(index));
        }
    }
    if (success || err != ERROR_SUCCESS)
    {
        free_operation(index);
    }
    return err;
}


tas_error 
tas_overlapped_server::write_file(
    tas_handle _handle, 
    char * _buf, 
    tas_size _size, 
    tas_operation_callback _callback, 
    void * _exdata) noexcept
{
    tas_size index = attach_operation(_handle, _buf, _size, _callback, _exdata);
    if (index == sc_invalid_index)
    {
        return is_aborted() ? ERROR_OPERATION_ABORTED : ERROR_IO_PENDING;
    }
    tas_error err = ERROR_SUCCESS;
    DWORD written = 0;
    bool success = WriteFile(
        file(index),
        operation(index)->buffer,
        operation(index)->size,
        &written,
        overlapped(index)->raw()
    );
    err = GetLastError();
    if (success && (written == operation(index)->size))
    {
        operation(index)->written = written;
        SetEvent(event(index));
    }
    else if (!success && (err == ERROR_IO_PENDING))
    {
        err = ERROR_SUCCESS;
    }
    else 
    {
        free_operation(index);
    }
    return err;
}


tas_error
tas_overlapped_server::read_file(
    tas_handle _handle, 
    char * _buf, 
    tas_size _size, 
    tas_operation_callback _callback, 
    void * _exdata) noexcept
{
    tas_size index = attach_operation(_handle, _buf, _size, _callback, _exdata);
    if (index == sc_invalid_index)
    {
        return is_aborted() ? ERROR_OPERATION_ABORTED : ERROR_IO_PENDING;
    }
    tas_error err = ERROR_SUCCESS;
    DWORD written = 0;
    bool success = ReadFile(
        file(index),
        operation(index)->buffer,
        operation(index)->size,
        &operation(index)->written,
        overlapped(index)->raw()
    );
    err = GetLastError();
    if (success && (written != 0))
    {
        SetEvent(event(index));
    }
    else if (!success && (err == ERROR_IO_PENDING))
    {
        err = ERROR_SUCCESS;
    }
    else
    {
        free_operation(index);
    }
    return err;
}


tas_error 
tas_overlapped_server::wait(
    tas_size _timeout) noexcept
{
    tas_error err = ERROR_SUCCESS;    
    if (tas_operation * operation = wait_aux(_timeout, err))
    {
        operation->callback(*this, *operation, err);
    }
    return err;
}


bool 
tas_overlapped_server::is_aborted() noexcept
{
    return m_aborted;
}


tas_error 
tas_overlapped_server::abort_and_wait_all() noexcept
{
    tas_error last_error = ERROR_SUCCESS;
    m_aborted = true;
    cancel_all();
    while (m_events_count != 0)
    {
        last_error = wait(200);
        if (last_error != EXIT_SUCCESS && last_error != ERROR_TIMEOUT)
        {
            break;
        }
    }
    m_aborted = (last_error != ERROR_SUCCESS);
    return last_error;
}


void 
tas_overlapped_server::cancel_all() noexcept
{    
    for (tas_size i = 0; i < m_events_count; ++i)
    {
        CancelIoEx(file(i), overlapped(i)->raw());
    }
}

inline tas_size 
tas_overlapped_server::events_count() noexcept
{
    return m_events_count;
}

inline tas_handle * 
tas_overlapped_server::events_array() noexcept
{
    return m_events;
}

inline tas_operation * 
tas_overlapped_server::operation(
    tas_size _i) noexcept
{
    return m_operations[_i];
}

inline tas_handle 
tas_overlapped_server::event(
    tas_size _i) noexcept
{
    return overlapped(_i)->event();
}


inline tas_handle 
tas_overlapped_server::file(
    tas_size _i) noexcept
{
    return operation(_i)->handle;
}

inline tas_overlapped * 
tas_overlapped_server::overlapped(
    tas_size _i) noexcept
{
    return m_overlappeds[_i];
}

tas_size 
tas_overlapped_server::attach_operation(
    tas_handle _handle, 
    char * _buf, 
    tas_size _size, 
    tas_operation_callback _callback, 
    void * _exdata) noexcept
{
    tas_size result = sc_invalid_index;
    if (m_events_count < sc_max_handles && !m_aborted)
    {
        tas_operation * op = nullptr;
        result = m_events_count++;
        m_events[result] = event(result);
        op = operation(result);
        op->handle = _handle;
        op->buffer = _buf;
        op->size = _size;
        op->exdata = _exdata;
        op->callback = _callback;
    }
    return result;
}

tas_operation * 
tas_overlapped_server::free_operation(
    tas_size _i) noexcept
{
    --m_events_count;
    std::swap(m_events[_i], m_events[m_events_count]);
    std::swap(m_operations[_i], m_operations[m_events_count]);
    std::swap(m_overlappeds[_i], m_overlappeds[m_events_count]);
    return m_operations[m_events_count];
}

tas_operation * 
tas_overlapped_server::wait_aux(
    tas_size _timeout, 
    tas_error & _e) noexcept
{
    tas_operation * result = nullptr;
    DWORD waitResult = WaitForMultipleObjects(
        events_count(),
        events_array(),
        FALSE,
        _timeout
    );
    DWORD index = waitResult - WAIT_OBJECT_0;
    if (index < events_count())
    {
        _e = overlapped(index)->get_result(operation(index)->handle, operation(index)->written);
        result = free_operation(index);
    }
    else if (waitResult == WAIT_TIMEOUT)
    {
        _e = ERROR_TIMEOUT;
    }
    else
    {
        _e = GetLastError();
    }
    return result;
}

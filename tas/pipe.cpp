#include <Windows.h>
#include "pipe.h"


namespace
{
    DWORD make_open_mode(unsigned mode) noexcept
    {
        return FILE_FLAG_OVERLAPPED |
            ((mode & TAS_NP_MODE_IN) ? PIPE_ACCESS_INBOUND : 0) |
            ((mode & TAS_NP_MODE_OUT) ? PIPE_ACCESS_OUTBOUND : 0)
        ;
    }

    DWORD make_mode(unsigned mode) noexcept
    {
        return ((mode & TAS_NP_MODE_WMSG) ? PIPE_TYPE_MESSAGE : PIPE_TYPE_BYTE) |
            ((mode & TAS_NP_MODE_RMSG) ? PIPE_READMODE_MESSAGE : PIPE_READMODE_BYTE) |
            ((mode & TAS_NP_MODE_ALLOW_REMOTE) ? PIPE_ACCEPT_REMOTE_CLIENTS : PIPE_REJECT_REMOTE_CLIENTS)
        ;
    }
}//internal linkage 



tas_named_pipe::tas_named_pipe(tas_named_pipe && _src) noexcept:
    m_handle(_src.m_handle),
    m_options(_src.m_options)
{
    _src.m_handle = INVALID_HANDLE_VALUE;
}


tas_named_pipe & 
tas_named_pipe::operator=(tas_named_pipe && _src) noexcept
{
    if (this != &_src)
    {
        close();
        m_handle = _src.m_handle;
        m_options = _src.m_options;
        _src.m_handle = INVALID_HANDLE_VALUE;
    }
    return *this;
}

tas_named_pipe::tas_named_pipe():
    m_handle(INVALID_HANDLE_VALUE),
    m_options{}
{
}

tas_named_pipe::~tas_named_pipe()
{
    close();
}

tas_error 
tas_named_pipe::create(wchar_t const * _name, tas_pipe_options const & _options) noexcept
{
    if (valid())
    {
        return ERROR_INVALID_OPERATION;
    }
    m_handle = CreateNamedPipeW(
        _name, 
        make_open_mode(_options.mode), 
        make_mode(_options.mode), 
        _options.max_instances, 
        _options.sys_out_buffer_size, 
        _options.sys_in_buffer_size, 
        100, 
        NULL
    );
    return (m_handle != INVALID_HANDLE_VALUE) ? ERROR_SUCCESS : GetLastError();
}


tas_error 
tas_named_pipe::disconnect() noexcept
{
    return DisconnectNamedPipe(m_handle) ? ERROR_SUCCESS : GetLastError();
}


void 
tas_named_pipe::close() noexcept
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        disconnect();
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}


bool 
tas_named_pipe::valid() noexcept
{
    return m_handle != INVALID_HANDLE_VALUE;
}


tas_handle 
tas_named_pipe::handle() noexcept
{
    return m_handle;
}

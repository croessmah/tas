#include <Windows.h>
#include <type_traits>
#include "overlapped.h"


tas_overlapped::tas_overlapped() noexcept:
    tas_overlapped(false)
{
}


tas_overlapped::tas_overlapped(bool _state) noexcept:
    m_storage{}
{
    static_assert(sizeof(OVERLAPPED) <= sizeof(*this), "Implementation buffer is too small");
    raw()->hEvent = CreateEvent(NULL, TRUE, _state, NULL);
}


tas_overlapped::~tas_overlapped() noexcept
{
    auto aux = raw();
    if (aux->hEvent)
    {
        CloseHandle(aux->hEvent);
    }
}


tas_handle 
tas_overlapped::event() const noexcept
{
    return raw()->hEvent;
}


bool 
tas_overlapped::is_valid_event() const noexcept
{
    return raw()->hEvent != NULL;
}


tas_error 
tas_overlapped::get_result(
    tas_handle _file, 
    tas_size & _written, 
    bool _wait) noexcept
{
    if (GetOverlappedResult(_file, raw(), &_written, _wait))
    {
        return EXIT_SUCCESS;
    }
    return GetLastError();
}


_OVERLAPPED * 
tas_overlapped::raw() noexcept
{
    return reinterpret_cast<_OVERLAPPED *>(std::addressof(m_storage));
}

struct _OVERLAPPED const * 
tas_overlapped::raw() const noexcept
{
    return reinterpret_cast<_OVERLAPPED const *>(std::addressof(m_storage));
}
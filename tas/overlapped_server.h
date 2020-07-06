#pragma once
#ifndef INCLUDE_TAS_OVERLAPPED_SERVER_H
#define INCLUDE_TAS_OVERLAPPED_SERVER_H
#include <utility>
#include "types.h"
#include "overlapped.h"
#include "operation.h"


class tas_overlapped_server
{
public:
    tas_overlapped_server(tas_overlapped_server const &) = delete;
    tas_overlapped_server & operator=(tas_overlapped_server const &) = delete;
    tas_overlapped_server() noexcept;
    ~tas_overlapped_server() noexcept;
    bool create_success() noexcept;
    tas_error connect_named_pipe(tas_handle _handle, char * _buf, tas_size _size, tas_operation_callback _callback, void * _exdata) noexcept;
    tas_error write_file(tas_handle _handle, char * _buf, tas_size _size, tas_operation_callback _callback, void * _exdata) noexcept;
    tas_error read_file(tas_handle _handle, char * _buf, tas_size _size, tas_operation_callback _callback, void * _exdata) noexcept;
    tas_error wait(tas_size _timeout) noexcept;
    bool is_aborted() noexcept;
    tas_error abort_and_wait_all() noexcept;
    void cancel_all() noexcept;
private: 
    
    tas_size events_count() noexcept;
    tas_handle * events_array() noexcept;
    tas_operation * operation(tas_size _i) noexcept;
    tas_handle event(tas_size _i) noexcept;
    tas_handle file(tas_size _i) noexcept;
    tas_overlapped * overlapped(tas_size _i) noexcept;

    tas_size attach_operation(tas_handle _handle, char * _buf, tas_size _size, tas_operation_callback _callback, void * _exdata) noexcept;
    tas_operation * free_operation(tas_size _i) noexcept;
    tas_operation * wait_aux(tas_size _timeout, tas_error & _e) noexcept;


    constexpr static tas_size sc_invalid_index = ~tas_size(0u);
    constexpr static tas_size sc_max_handles = 32u;
    tas_overlapped m_overlappeds_storage[sc_max_handles];
    tas_operation m_operations_storage[sc_max_handles];
    tas_operation * m_operations[sc_max_handles];
    tas_overlapped * m_overlappeds[sc_max_handles];
    tas_handle m_events[sc_max_handles];
    tas_size m_events_count;
    bool m_aborted;
};

#endif //INCLUDE_TAS_OVERLAPPED_SERVER_H
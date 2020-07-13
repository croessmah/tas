#pragma once
#ifndef INCLUDE_TAS_CLIENT_H
#define INCLUDE_TAS_CLIENT_H
#include "pipe.h"


class tas_client
{
public:
    static constexpr unsigned sc_in_buffer_size = 1024;
    static constexpr unsigned sc_out_buffer_size = 4096;
    static constexpr wchar_t const * sc_pipes_name = L"\\\\.\\pipe\\tas_client";
    static constexpr tas_pipe_options sc_pipes_options =
    {
        5,
        1024,
        2048,
        TAS_NP_MODE_INOUT | TAS_NP_MODE_WMSG | TAS_NP_MODE_RMSG
    };
    tas_client(tas_controller * _ctrls, unsigned _count) noexcept;
    ~tas_client() noexcept;
    bool to_work(tas_overlapped_server & _osrv) noexcept;
    void close_channel() noexcept;
    bool reaccept(tas_overlapped_server & _osrv) noexcept;
    bool connected() const noexcept;
    tas_size last_time() const noexcept;
    tas_error last_error() const noexcept;
private:
    tas_size process_query(tas_size _query_size) const noexcept;
    bool read(tas_overlapped_server & _osrv) noexcept;
    bool write(tas_overlapped_server & _osrv, tas_size _count) noexcept;
    void disconnect() noexcept;
    static void callback_connected(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);
    static void callback_read(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);
    static void callback_write(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);

    using pipe_in_buffer = char[sc_in_buffer_size];
    using pipe_out_buffer = char[sc_out_buffer_size];
    tas_controller * m_controllers;
    unsigned m_controllers_count;
    tas_named_pipe m_pipe;
    tas_size m_last_update_time;
    tas_error m_last_error;
    bool m_connected;
    pipe_in_buffer m_in_buffer;
    pipe_out_buffer m_out_buffer;
};


#endif //INCLUDE_TAS_CLIENT_H
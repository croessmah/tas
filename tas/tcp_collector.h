#pragma once
#ifndef INCLUDE_TAS_TCP_COLLECTOR_H
#define INCLUDE_TAS_TCP_COLLECTOR_H

#include <cstdint>
#include "tcp_header.h"


class tas_tcp_collector
{
public:
    static constexpr unsigned sc_max_buffer_size = 2048;
    tas_tcp_collector() noexcept;
    char * data() noexcept { return m_buffer; }
    unsigned size() noexcept { return m_buffer_size; }
    unsigned lost() noexcept { return sc_max_buffer_size - m_buffer_size; }
    bool update(char const * _tcp_frame, uint32_t _size) noexcept;
private:
    using state_function_t = bool (tas_tcp_collector::*)();
    bool append() noexcept;
    
    bool state_invalid() noexcept;
    bool state_wait_sync() noexcept;
    bool state_sync() noexcept;
    bool state_work() noexcept;
    bool state_pushed() noexcept;
    bool update_header(char const * _tcp_frame, uint32_t _size) noexcept;
    bool check_invalidate_packet() noexcept;
    bool check_violation_ack_seq_nums() noexcept;
    void update_next_ack_num() noexcept;
    void update_next_seq_num() noexcept;
    uint32_t tcp_data_size() noexcept;
    tas_tcp_header m_hdr;
    char const * m_tcp_frame_begin;
    uint32_t m_tcp_frame_size;
    uint32_t m_next_ack;
    uint32_t m_next_seq;
    char m_buffer[sc_max_buffer_size];
    uint32_t m_buffer_size;
    unsigned m_state;
};


#endif //INCLUDE_TAS_TCP_COLLECTOR_H
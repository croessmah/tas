#pragma once
#include <cstdint>
#include "tcp_header.h"


class tas_tcp_collector
{
public:
    tas_tcp_collector(char * _buffer, uint32_t _size) noexcept;
    char * data() noexcept { return m_stream_begin; }
    unsigned size() noexcept { return m_stream_size; }
    unsigned lost() noexcept { return m_stream_capacity - m_stream_size; }
    unsigned set_buffer(char * _buffer, uint32_t _size) noexcept;
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
    char * m_stream_begin;
    uint32_t m_stream_size;
    uint32_t m_stream_capacity;
    unsigned m_state;
};


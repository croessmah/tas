#include <Windows.h>
#include <cassert>
#include "tcp_collector.h"



enum eTCP_COLLECTOR_STATES : unsigned
{
    TCP_CS_INVALID,
    TCP_CS_WAIT_SYNC,
    TCP_CS_SYNC,
    TCP_CS_WORK,
    TCP_CS_PUSHED,
    TCP_CS_COUNT
};

tas_tcp_collector::tas_tcp_collector() noexcept:
    m_buffer{},
    m_buffer_size(0),
    m_state(TCP_CS_INVALID)
{
}


bool tas_tcp_collector::update(char const * _tcp_frame, uint32_t _size) noexcept
{    
    static const state_function_t fs_table[TCP_CS_COUNT] =
    {
        &tas_tcp_collector::state_invalid,
        &tas_tcp_collector::state_wait_sync,
        &tas_tcp_collector::state_sync,
        &tas_tcp_collector::state_work,
        &tas_tcp_collector::state_pushed
    };

    assert(m_state < TCP_CS_COUNT && "Unexpected value of state");

    update_header(_tcp_frame, _size);

    while ((this->*fs_table[m_state])())
    {
    }

    return m_state == TCP_CS_PUSHED;
}


bool tas_tcp_collector::append() noexcept
{
    unsigned size = tcp_data_size();
    char const * data = m_tcp_frame_begin + m_hdr.full_header_length();
    if (lost() < size)
    {
        return false;
    }
    memcpy(m_buffer + m_buffer_size, data, size);
    m_buffer_size += size;
    return true;
}


bool inline tas_tcp_collector::state_invalid() noexcept
{
    m_buffer_size = 0;
    m_state = TCP_CS_WAIT_SYNC;
    return true;
}


bool inline tas_tcp_collector::state_wait_sync() noexcept
{
    if (m_hdr.psh())
    {
        m_state = TCP_CS_SYNC;
        update_next_seq_num();
    }
    return false;
}


bool inline tas_tcp_collector::state_sync() noexcept
{
    if (m_hdr.ack())
    {
        update_next_ack_num();
        m_state = TCP_CS_WORK;
        return true;
    }
    m_state = TCP_CS_INVALID;
    return false;
}


bool inline tas_tcp_collector::state_work() noexcept
{
    if (check_invalidate_packet() || check_violation_ack_seq_nums())
    {
        m_state = TCP_CS_INVALID;
        return true;
    }
    if (!append())
    {
        assert(false && "buffer is too small");
        //todo: log error
        m_state = TCP_CS_INVALID;
        return true;
    }
    if (m_hdr.psh())
    {
        m_state = TCP_CS_PUSHED;
    }
    else
    {
        update_next_seq_num();
    }
    return false;
}


bool inline tas_tcp_collector::state_pushed() noexcept
{
    m_state = TCP_CS_INVALID;
    return true;
}

bool tas_tcp_collector::update_header(char const * _tcp_frame, uint32_t _size) noexcept
{
    if (_size < sizeof(m_hdr))
    {
        return false;
    }
    memcpy(&m_hdr, _tcp_frame, sizeof(m_hdr));
    if (_size < m_hdr.full_header_length())
    {
        return false;
    }
    m_tcp_frame_begin = _tcp_frame;
    m_tcp_frame_size = _size;
    return true;
}

bool tas_tcp_collector::check_invalidate_packet() noexcept
{
    return m_hdr.fin() || m_hdr.rst() || m_hdr.syn();
}

bool tas_tcp_collector::check_violation_ack_seq_nums() noexcept
{
    return m_hdr.ack_num() != m_next_ack || m_hdr.seq_num() != m_next_seq;
}

void tas_tcp_collector::update_next_ack_num() noexcept
{
    m_next_ack = m_hdr.ack_num();
}

void tas_tcp_collector::update_next_seq_num() noexcept
{
    m_next_seq = htonl(ntohl(m_hdr.seq_num()) + tcp_data_size());
}

uint32_t tas_tcp_collector::tcp_data_size() noexcept
{
    return m_tcp_frame_size - m_hdr.full_header_length();
}

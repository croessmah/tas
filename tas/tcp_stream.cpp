#include <Windows.h>
#include "tcp_stream.h"


#include <iostream>

void tas_tcp_stream::set_buffer(char * _buffer, unsigned _size) noexcept
{
    m_stream_begin = _buffer;
    m_stream_capacity = _size;
    m_stream_size = 0;
}


void tas_tcp_stream::reset() noexcept
{
    m_ack_sync = false;
    m_stream_size = 0;     
    m_pushed = false;
}

void tas_tcp_stream::reset_header() noexcept
{
    m_previous_hdr = tas_tcp_header{};
}

bool tas_tcp_stream::push(char const * _data, unsigned _size) noexcept
{
    if (lost() < _size)
    {
        return false;
    }
    memcpy(m_stream_begin + m_stream_size, _data, _size);
    m_stream_size += _size;
    return true;
}

bool tas_tcp_stream::update(char const * _tcp_frame, unsigned _size) noexcept
{
    //std::cout << "\nbegin update\n";
    tas_tcp_header hdr;
    if (_size < sizeof(hdr))
    {
        reset();
        return false;
    }
    memcpy(&hdr, _tcp_frame, sizeof(hdr));
    if (_size < hdr.full_header_length() || hdr.fin() || hdr.rst() || hdr.syn())
    {
        reset();
        return false;
    }
        
    if (m_pushed)
    {
        m_stream_size = 0;
        m_pushed = false;
        m_ack_sync = false;
    }

    bool sync_packet = false;
    if (!m_ack_sync && m_previous_hdr.psh())
    {
        m_ack_sync = true;
        sync_packet = true;
    }
    /*std::cout << "asc sync: " << m_ack_sync << "\n";*/
    unsigned user_data_size = _size - hdr.full_header_length();
    if (m_ack_sync)
    {        
        bool ack_success = sync_packet || (hdr.ack() && hdr.ack_num() == m_previous_hdr.ack_num());
        bool seq_success = hdr.seq_num() == m_next_seq_num;
        /*std::cout << "seq_num: " << ntohl(hdr.seq_num())
            << " prev seq_num: " << ntohl(m_previous_hdr.seq_num()) << "\n";
        std::cout << "ack_success: " << ack_success << "\n";
        std::cout << "seq_success: " << seq_success << "\n";*/
        if (ack_success && seq_success)
        {
            if (push(_tcp_frame + hdr.full_header_length(), user_data_size))
            {
                m_pushed = hdr.psh();
                m_previous_hdr = hdr;
                m_next_seq_num = htonl(ntohl(hdr.seq_num()) + user_data_size);
            }
            else
            {
                reset();
                reset_header();
            }
            return m_pushed;
        }
    }
    reset();
    m_previous_hdr = hdr;
    m_next_seq_num = htonl(ntohl(hdr.seq_num()) + user_data_size);
    return false;
}

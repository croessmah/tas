#pragma once
#ifndef INCLUDE_TAS_TCP_STREAM_H
#define INCLUDE_TAS_TCP_STREAM_H

#include "tcp_header.h"



class tas_tcp_stream
{
public:
    tas_tcp_stream(char * _buffer, unsigned _size) noexcept:
        m_stream_begin(_buffer),
        m_stream_size(0),
        m_stream_capacity(_size),
        m_ack_sync(false),
        m_pushed(false)
    {}
    
    char * data() noexcept { return m_stream_begin; }
    unsigned size() noexcept { return m_stream_size; }
    unsigned lost() noexcept { return m_stream_capacity - m_stream_size; }
    void set_buffer(char * _buffer, unsigned _size) noexcept;
    explicit operator bool() noexcept { return m_pushed; }
    bool update(char const * _tcp_frame, unsigned _size) noexcept;
private:
    void reset() noexcept;
    void reset_header() noexcept;
    bool push(char const * _data, unsigned _size) noexcept;    
    tas_tcp_header m_previous_hdr;
    char * m_stream_begin;
    unsigned m_stream_size;
    unsigned m_stream_capacity;
    unsigned m_next_seq_num;
    bool m_ack_sync;
    bool m_pushed;
};






#endif //INCLUDE_TAS_TCP_STREAM_H
#pragma once
#ifndef INCLUDE_TAS_CONTROLLER_H
#define INCLUDE_TAS_CONTROLLER_H

#include <cstdint>
#include "tcp_stream.h"

class tas_controller
{
public:
    tas_controller(char const * _name, uint32_t _ip) noexcept;
    bool update(char const * _data, unsigned _size) noexcept;
    uint32_t ip() noexcept;
private:
    bool process_e2(char const *& _data, unsigned & _size) noexcept;
    bool process_ip(char const *& _data, unsigned & _size) noexcept;
    bool process_tcp(char const * _data, unsigned _size) noexcept;
    bool process_data(char const *& _data, unsigned & _size) noexcept;

    static constexpr unsigned sc_buffer_size = 2048;
    uint32_t m_ip;
    unsigned m_front_buffer_size;
    char * m_front_buffer;
    char * m_back_buffer;
    tas_tcp_stream m_stream;
    char m_name[8];
    char m_buffer[sc_buffer_size * 2];
};


#endif //INCLUDE_TAS_CONTROLLER_H
#pragma once
#ifndef INCLUDE_TAS_CONTROLLER_H
#define INCLUDE_TAS_CONTROLLER_H

#include <cstdint>
#include "tcp_collector.h"

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
    void process_data() noexcept;
    void update_values() noexcept;
    static constexpr unsigned sc_raw_buffer_size = 2048;
    uint32_t m_ip;
    tas_tcp_collector m_collector;
    char m_name[8];
};


#endif //INCLUDE_TAS_CONTROLLER_H
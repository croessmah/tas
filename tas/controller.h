#pragma once
#ifndef INCLUDE_TAS_CONTROLLER_H
#define INCLUDE_TAS_CONTROLLER_H

#include <cstdint>
#include "tcp_collector.h"
#include "packet_values.h"


class tas_cdx
{
public:
    tas_cdx(uint16_t _cdx): m_idx(_cdx) {}
    tas_cdx(uint16_t _pdx, uint16_t _vdx) noexcept : m_idx((_pdx << 10) | (_vdx & 0x3FF)) {}
    uint16_t pdx() noexcept { return m_idx >> 10; }
    uint16_t vdx() noexcept { return m_idx & 0x3FF; }
private:
    uint16_t m_idx;
};


class tas_controller
{
public:
    tas_controller(char const * _ip) noexcept;
    tas_controller(uint32_t _ip) noexcept;
    bool update(char const * _data, unsigned _size) noexcept;
    uint32_t ip() noexcept;
    char const * value(tas_cdx _cdx) noexcept
    {
        return (_cdx.pdx() < sc_max_packets_count) ?
            m_packets[_cdx.pdx()].value(_cdx.vdx()) : 
            nullptr;
    }

    uint64_t update_lost_ms() noexcept;
private:    
    bool process_e2(char const *& _data, unsigned & _size) noexcept;
    bool process_ip(char const *& _data, unsigned & _size) noexcept;
    bool process_tcp(char const * _data, unsigned _size) noexcept;
    void process_data() noexcept;
    static constexpr unsigned sc_max_packets_count = 3;
    uint32_t m_ip;
    uint64_t m_last_update;
    tas_tcp_collector m_collector;
    tas_packet_values m_packets[sc_max_packets_count];
};


#endif //INCLUDE_TAS_CONTROLLER_H
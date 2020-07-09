#pragma once
#ifndef INCLUDE_TAS_TCP_HEADER_H
#define INCLUDE_TAS_TCP_HEADER_H
#include <cstdint>

class tas_tcp_header
{
public:
    uint16_t src_port() noexcept { return m_data[0] & 0xFFFF; }
    uint16_t dst_port() noexcept { return m_data[0] >> 16; }
    uint32_t seq_num() noexcept { return m_data[1]; }
    uint32_t ack_num() noexcept { return m_data[2]; }
    uint32_t full_header_length() noexcept { return 4 * ((m_data[3] & 0xF0) >> 4); }
    uint8_t flags() noexcept { return uint8_t((m_data[3] >> 8) & 0x3F); }
    bool urg() noexcept { return flags() & 0b100000; }
    bool ack() noexcept { return flags() & 0b10000; }
    bool psh() noexcept { return flags() & 0b1000; }
    bool rst() noexcept { return flags() & 0b100; }
    bool syn() noexcept { return flags() & 0b10; }
    bool fin() noexcept { return flags() & 0b1; }
private:
    uint32_t m_data[5];
};

#endif //INCLUDE_TAS_TCP_HEADER_H
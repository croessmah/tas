#include <Windows.h>
#include <cstring>
#include <cassert>
#include "tcp_header.h"
#include "controller.h"


tas_controller::tas_controller( uint32_t _ip) noexcept:
    m_ip(_ip),
    m_last_update(0)
{
}


bool tas_controller::update(char const * _data, unsigned _size) noexcept
{
    if (process_e2(_data, _size) && process_ip(_data, _size))
    {
        if (process_tcp(_data, _size))
        {
            process_data();
        }
        return true;
    }
    return false;
}


uint32_t tas_controller::ip() noexcept
{
    return m_ip;
}

uint64_t tas_controller::update_lost_ms() noexcept
{
    if (m_last_update == 0)
    {
        return UINT64_MAX;
    }
    return GetTickCount64() - m_last_update;
}

bool tas_controller::process_e2(char const *& _data, unsigned & _size) noexcept
{
    struct ethernet2_hdr
    {
        bool IsIp() const noexcept { return ethType == 0x0008; }
        char dstMac[6];
        char srcMac[6];
        uint16_t ethType;
    } hdr;
    static_assert (sizeof(hdr) == 14, "Invalid Ethernet2Header struct");
    if (sizeof(hdr) <= _size) {
        memcpy(&hdr, _data, sizeof(hdr));
        if (hdr.IsIp()) {
            _data += sizeof(hdr);
            _size -= sizeof(hdr);
            return true;
        }
    } 
    return false;
}

bool tas_controller::process_ip(char const *& _data, unsigned & _size) noexcept
{
    struct ipv4_hdr
    {
        uint8_t GetVersion() const { return (rawData[0] & 0xF0u) >> 4u; }
        uint16_t GetTotalLength() const { return uint16_t((rawData[0] & 0xFFFF0000u) >> 16u); }
        uint8_t GetHeaderSize() const { return rawData[0] & 0x0Fu; }
        uint8_t GetProtocol() const { return (rawData[2] & 0xFF00u) >> 8u; }
        uint32_t GetSourceIp() const { return uint32_t(rawData[3]); }
        uint32_t GetDestinationIp() const { return uint32_t(rawData[4]); }
        bool IsIpv4() const { return GetVersion() == 0x04u; }
        uint32_t rawData[5];
    } hdr;

    static_assert (sizeof(hdr) == 20, "Invalid ipv4_hdr struct");
    if (sizeof(hdr) <= _size) {
        memcpy(&hdr, _data, sizeof(hdr));
        if (hdr.IsIpv4() &&  hdr.GetSourceIp() == m_ip) {
            uint32_t header_size = hdr.GetHeaderSize() * 4;
            uint32_t total_length = ntohs(hdr.GetTotalLength());
            if (total_length <= _size && header_size <= total_length)
            {
                _size = total_length - header_size;
                _data += header_size;
                return true;
            }
        }
    }
    return false;
}

bool tas_controller::process_tcp(char const * _data, unsigned _size) noexcept
{

    return m_collector.update(_data, _size);;
}

void tas_controller::process_data() noexcept
{
    tas_packet_values packet;
    if (packet.parse(m_collector.data(), m_collector.size()) && 
        packet.pdx() < sc_max_packets_count)
    {
        m_packets[packet.pdx()] = packet;
        m_last_update = GetTickCount64();
    }
}

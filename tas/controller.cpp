#include <Windows.h>
#include <cstring>
#include <cassert>
#include "tcp_header.h"
#include "controller.h"


tas_controller::tas_controller(char const * _name, uint32_t _ip) noexcept:
    m_ip(_ip),
    m_front_buffer(m_buffer),
    m_back_buffer(m_buffer + sc_buffer_size),
    m_collector(m_back_buffer, sc_buffer_size),
    m_front_buffer_size(0)
{
    assert(strlen(_name) < sizeof(m_name) && "name buffer is too small");
    strcpy(m_name, _name);
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
    tas_tcp_header hdr;/*
    if (sizeof(hdr) <= _size)
    {
        memcpy(&hdr, _data, sizeof(hdr));
        if (hdr.full_header_length() <= _size)
        {
            std::cout
                << "asc num: " << ntohl(hdr.ack_num()) << " "
                << "seq num: " << ntohl(hdr.seq_num()) << "\n"
                << "fin:" << hdr.fin() << " "
                << "rst:" << hdr.rst() << " "
                << "syn:" << hdr.syn() << " "
                << "ack:" << hdr.ack() << " "
                << "psh:" << hdr.psh() << " ";
        }
    }*/
    bool completed = m_collector.update(_data, _size);
    if (completed)
    {
        m_front_buffer_size = m_collector.size();
        char * tmp = m_front_buffer;
        m_front_buffer = m_back_buffer;
        m_back_buffer = tmp;
        m_collector.set_buffer(m_back_buffer, sc_buffer_size);
    }
    return completed;
}

void tas_controller::process_data() noexcept
{
    if (m_front_buffer_size != 0)
    {
        //todo: find from end
        m_front_buffer_size = strlen(m_front_buffer);
    }
    //(std::cout << "tcp data size: " << m_front_buffer_size << "\n").write(m_front_buffer, m_front_buffer_size);
}

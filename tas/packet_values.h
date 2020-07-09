#pragma once
#ifndef INCLUDE_TAS_PACKET_VALUES_H
#define INCLUDE_TAS_PACKET_VALUES_H


static constexpr unsigned gc_invalid_pdx= ~0u;
class tas_packet_values
{
public:
    tas_packet_values() : m_values_count(0) {}
    unsigned pdx() noexcept;
    char const * value(unsigned _vdx) noexcept
    {
        return (m_values_count < _vdx) ? m_values[_vdx].str : nullptr;
    }

    bool parse(char const * _data, unsigned _size) noexcept;
private:
    bool parse_aux(char const * _data, unsigned _size) noexcept;
    static constexpr unsigned sc_max_values_count = 300;
    static constexpr unsigned sc_max_value_size = 8;
    unsigned m_values_count;
    struct raw_value
    {
        char str[sc_max_value_size];
    };
    raw_value m_values[sc_max_values_count];
};



#endif //INCLUDE_TAS_PACKET_VALUES_H
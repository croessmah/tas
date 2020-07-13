#include <cstring>
#include <cassert>
#include "packet_values.h"


namespace
{
    unsigned cvt_pdx(char const * _str) noexcept
    {
        unsigned result = 0;
        while (*_str)
        {
            if (*_str < '0' || *_str > '9')
            {
                return gc_invalid_pdx;
            }
            result *= 10;
            result += *_str - '0';
            ++_str;
        }
        return result / 10;
    }

}//internal linkage



unsigned tas_packet_values::pdx() noexcept
{
    if (m_values_count)
    {
        return cvt_pdx(m_values[0].str);
    }
    return gc_invalid_pdx;
}


bool tas_packet_values::parse(char const * _data, unsigned _size) noexcept
{
    if (parse_aux(_data, _size) && pdx() != gc_invalid_pdx)
    {
        return true;
    }
    m_values_count = 0;
    return false;
}


bool tas_packet_values::parse_aux(char const * _data, unsigned _size) noexcept
{
    m_values_count = 0;
    //"<0#10;1#456;...;N#445.55;>"
    if (_size == 0 || *_data != '<' || _data[_size - 1] != '\0')
    {
        return false;
    }
    while (_size && _data[_size - 1] == '\0')
    {
        --_size;
    }
    if (_size == 0 || _data[_size - 1] != '>')
    {
        return false;
    }
    ++_data;//skip '<'
    --_size;//skip '>'
    char const * begin = strchr(_data, '#');
    if (begin == nullptr)
    {
        return false;
    }
    while (begin)
    {
        ++begin;
        char const * end = strchr(begin, ';');
        if (end == nullptr)
        {
            return false;
        }
        unsigned size = end - begin;
        assert("Values buffer is too small" && m_values_count < sc_max_values_count);
        if (!(m_values_count < sc_max_values_count))
        {
            return false;
        }
        if (!(size < sc_max_value_size))
        {
            size = sc_max_value_size - 1;
        }
        memcpy(m_values[m_values_count].str, begin, size);
        m_values[m_values_count].str[size] = '\0';
        ++m_values_count;
        begin = strchr(end + 1, '#');
    }
    return true;
}

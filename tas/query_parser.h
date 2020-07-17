#pragma once
#ifndef INCLUDE_TAS_QUERY_PARSER_H
#define INCLUDE_TAS_QUERY_PARSER_H
#include <cstdint>


class tas_query_parser
{
public:
    tas_query_parser(class tas_controller * _controllers, unsigned _count) noexcept;
    unsigned parse(char const * _in, unsigned _in_size, char * _out, unsigned _out_size) noexcept;
private:
    bool parse_index() noexcept;
    bool parse_cdx() noexcept;
    bool process_cdx() noexcept;
    bool process_index() noexcept;

    char const * m_in;
    char const * m_in_end;
    char * m_out_begin;
    char * m_out_end;
    char * m_out_end_of_storage;
    class tas_controller * m_controllers;
    unsigned m_controllers_count;
    int m_last_error;
    uint16_t m_cdx;
    uint16_t m_controller_index;
};


#endif //INCLUDE_TAS_QUERY_PARSER_H
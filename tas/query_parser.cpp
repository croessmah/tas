#include <cstring>
#include <charconv>
#include "controller.h"
#include "query_parser.h"

/*
* сообщение клиента с запросом параметров:
* Index cdx1 ... cdxN
*
* сообщение сервера с ответом:
* Index:sec cdx1:val ... cdxN:val
* sec - количество секунд, прошедшее с последнего обновления. Если данных еще нет - UINT64_MAX.
* val - текстовое представление значения длинной до 7-ми символов включительно, либо err
* ошибка формата:
* err:1
* слишком длинный ответ:
* err:2
*
* маскимальное число одновременно подключенных клиентов - sc_max_clients (5)
*/

namespace
{
    char const * const gc_errors_table[] = {
        "success",
        "invalid format",
        "ansfer overflow"
    };


    enum eParserErrors
    {
        QPE_SUCCESS,
        QPE_INVALID_FORMAT,
        QPE_OVERFLOW
    };


    char const * skip_spaces(char const * _begin, char const * _end) noexcept
    {
        while (_begin != _end && *_begin <= ' ')
        {
            ++_begin;
        }
        return _begin;
    }

}//internal linkage


tas_query_parser::tas_query_parser(tas_controller * _controllers, unsigned _count) noexcept:
    m_controllers(_controllers),
    m_controllers_count(_count)
{
}


unsigned tas_query_parser::parse(char const * _in, unsigned _in_size, char * _out, unsigned _out_size) noexcept
{
    m_in = _in;
    m_in_end = _in + _in_size;
    m_out_begin = _out;
    m_out_end = _out;
    m_out_end_of_storage = _out + _out_size;
    m_last_error = QPE_SUCCESS;
    if (parse_index())
    {
        while (parse_cdx())
        {
        }
    }
    if (m_last_error)
    {
        char const * const err_text = gc_errors_table[m_last_error];;
        unsigned err_text_length = strlen(err_text);
        memcpy(m_out_begin, err_text, err_text_length);
        m_out_end = m_out_begin + err_text_length;
    }
    return static_cast<unsigned>(m_out_end - m_out_begin);
}


bool tas_query_parser::parse_index() noexcept
{
    m_in = skip_spaces(m_in, m_in_end);
    if (auto[end, err] = std::from_chars(m_in, m_in_end, m_controller_index); 
        err == std::errc())
    {
        m_in = end;
        return process_index();
    }
    m_last_error = QPE_INVALID_FORMAT;
    return false;
}


bool tas_query_parser::parse_cdx() noexcept
{
    m_in = skip_spaces(m_in, m_in_end);
    if (m_in != m_in_end)
    {        
        if (auto[end, err] = std::from_chars(m_in, m_in_end, m_cdx); 
            err == std::errc())
        {
            m_in = end;
            return process_cdx();
        }
        m_last_error = QPE_INVALID_FORMAT;
    }
    return false;
}


bool tas_query_parser::process_cdx() noexcept
{
    char const * value = nullptr;
    if (m_controller_index < m_controllers_count)
    {
        value = m_controllers[m_controller_index].value(m_cdx);
    }

    if (m_out_end == m_out_end_of_storage)
    {
        m_last_error = QPE_OVERFLOW;
        return false;        
    }
    *(m_out_end++) = ' ';
    m_out_end = std::to_chars(m_out_end, m_out_end_of_storage, m_cdx).ptr;
    if (!value)
    {
        value = "err";
    }
    unsigned val_size = strlen(value);
    unsigned out_lost = m_out_end_of_storage - m_out_end;
    if (out_lost < (val_size + 1))
    {
        m_last_error = QPE_OVERFLOW;
        return false;
    }
    *(m_out_end++) = ':';
    memcpy(m_out_end, value, val_size);
    m_out_end += val_size;
    return true;
}


bool tas_query_parser::process_index() noexcept
{
    m_last_error = QPE_SUCCESS;
    m_out_end = std::to_chars(m_out_end, m_out_end_of_storage, m_controller_index).ptr;
    *(m_out_end++) = ':';
    if (m_controller_index < m_controllers_count)
    {
        uint64_t controller_update_lost_ms = m_controllers[m_controller_index].update_lost_ms();
        if (controller_update_lost_ms != UINT64_MAX)
        {
            m_out_end = std::to_chars(m_out_end, m_out_end_of_storage, controller_update_lost_ms).ptr;
            return true;
        }
    }
    m_out_end = std::to_chars(m_out_end, m_out_end_of_storage, -1).ptr;
    return true;
}

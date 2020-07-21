#include <Windows.h>
#include <charconv>
#include "request.h"


namespace
{
    static constexpr wchar_t const * sc_pipes_name = L"\\\\.\\pipe\\tas_client";

    DWORD call_pipe(std::string const & _request, DWORD _timeout, std::string & _ansfer)
    {
        char local_buffer[tas::request::sc_max_request_size * 5];
        DWORD written = 0;
        bool success = CallNamedPipeW(
            sc_pipes_name,
            const_cast<char *>(_request.data()),
            static_cast<unsigned>(_request.size()),
            local_buffer,
            sizeof(local_buffer),
            &written,
            _timeout
        );

        if (success)
        {
            _ansfer.assign(local_buffer, written);
            return ERROR_SUCCESS;
        }
        return GetLastError();
    }


    DWORD send_request(std::string const & _request, unsigned _timeout, std::string & _ansfer)
    {
        DWORD timeout = (_timeout == ::tas::request::sc_infinite) ? 
            static_cast<DWORD>(INFINITE) : 
            static_cast<DWORD>(_timeout) ;
        return call_pipe(_request, timeout, _ansfer);
    }
    

    void validate_ansfer(std::string const & _ansfer)
    {
        if (_ansfer.compare(0, 7, "fmtfail"))
        {
            throw ::tas::request_invalid("invalid request format");
        }
        else if (_ansfer.compare(0, 7, "msgfail"))
        {
            throw ::tas::request_overflow("message is too long");
        }
    }


}//internal linkage


namespace tas
{

    request::request()
    {
    }


    std::string const & request::send(unsigned _timeout, bool _repeat_if_busy)
    {
        m_last_ansfer_text.clear();
        DWORD last_time = GetTickCount();
        do
        {
            DWORD request_result = send_request(m_request_text, _timeout, m_last_ansfer_text);
            if (request_result == ERROR_SUCCESS)
            {
                return m_last_ansfer_text;
            }
            else if (request_result == ERROR_TIMEOUT)
            {
                break;//request_timedout exception
            }
            else if (request_result != ERROR_PIPE_BUSY)
            {
                throw request_no_connection("connection failed");
            }            
            if (_timeout != sc_infinite)
            {
                DWORD current_time = GetTickCount();
                unsigned delta = static_cast<unsigned>(current_time - last_time);
                _timeout -= (_timeout < delta) ? _timeout : delta;
                last_time = current_time;
            }
        } while (_repeat_if_busy || _timeout);
        throw request_timedout("request timedout");
    }


    void request::clear() noexcept
    {
        m_request_text.clear();
    }


    bool request::add_number(uint16_t _num)
    {
        char text_buffer[24];        
        char * begin = std::begin(text_buffer);
        if (!m_request_text.empty())
        {
            *(begin++) = ' ';
        }
        char * end = std::end(text_buffer);
        end = std::to_chars(begin, end, _num).ptr;
        unsigned size = static_cast<unsigned>(end - text_buffer);
        if (size <= (m_request_text.capacity() - m_request_text.size()))
        {
            m_request_text.append(text_buffer, size);
            return true;
        }
        return false;
    }


    request & request::set_request_text(std::string const & _source)
    {
        return swap(std::string(_source));
    }

    request & request::swap(std::string && _source)
    {
        if (_source.size() <= sc_max_request_size)
        {
            m_request_text.swap(_source);
            return *this;
        }
        throw request_overflow("message is too long");
    }

}//namespace tas
#include <Windows.h>
#include <array>
#include <charconv>
#include <cassert>
#include "query.h"
#include "validator.h"


namespace
{
    inline char const * skip_spaces(char const * _cur) noexcept
    {
        while (*_cur == ' ')
        {
            ++_cur;
        }
        return _cur;
    }


    inline char const * find_space_or_null(char const * _cur) noexcept
    {
        while (*_cur && *_cur != ' ')
        {
            ++_cur;
        }
        return _cur;
    }


    inline uint16_t make_cdx(uint16_t _pdx, uint16_t _vdx) noexcept
    {
        return (_pdx << uint16_t(10u)) | (_vdx & uint16_t(0x3FF));
    }


    void srgm_validate(uint16_t _pdx, uint16_t _cdx)
    {
        static tas::validator<3u> constexpr sc_validator =
        {
            {{10}, {10}, {10}},
        };
        sc_validator(_pdx, _cdx);
    }


    void ft5p_validate(uint16_t _pdx, uint16_t _cdx)
    {
        tas::validator<4u> constexpr sc_validator =
        {
            {{10}, {10}, {10}, {10}},
        };
        sc_validator(_pdx, _cdx);
    }
    
}//internal linkage




namespace tas
{
    struct query::param
    {
        std::string_view view;
        uint16_t cdx;

        friend bool inline operator<(query::param const & _p, uint16_t _cdx) noexcept
        {
            return _p.cdx < _cdx;
        }
        friend bool inline operator<(uint16_t _cdx, query::param const & _p) noexcept
        {
            return _cdx < _p.cdx;
        }
        friend bool inline operator==(query::param const & _p, uint16_t _cdx) noexcept
        {
            return _p.cdx == _cdx;
        }
        friend bool inline operator==(uint16_t _cdx, query::param const & _p) noexcept
        {
            return _cdx == _p.cdx;
        }
        friend bool inline operator!=(query::param const & _p, uint16_t _cdx) noexcept
        {
            return _p.cdx != _cdx;
        }
        friend bool inline operator!=(uint16_t _cdx, query::param const & _p) noexcept
        {
            return _cdx != _p.cdx;
        }

    };


    query::query(char const * _ctl_name):
        m_ctl_index(UINT16_MAX),
        m_validator(nullptr)
    {

        if (!strcmp(_ctl_name, "srgm"))
        {
            m_ctl_index = 0;
            m_validator = srgm_validate;
        }
        else if (!strcmp(_ctl_name, "ft5p"))
        {
            m_ctl_index = 1;
            m_validator = ft5p_validate;
        }
        else
        {
            throw query_invalid_ctl("invalid name of controller");
        }  
        clear_request();
    }

    query::~query()
    {
    }


    bool query::add_param(uint16_t _pdx, uint16_t _vdx)
    {
        //m_validator(_pdx, _vdx);
        uint16_t cdx = make_cdx(_pdx, _vdx);
        auto it = std::lower_bound(m_params.begin(), m_params.end(), cdx);
        if (it != m_params.end() && it->cdx == cdx)
        {
            return false;
        }
        m_params.insert(it, { std::string_view(), cdx });
        if (m_request.add_number(cdx))
        {
            return true;
        }
        throw request_overflow("request buffer overflow");
    }


    std::string_view query::get_param(uint16_t _pdx, uint16_t _vdx)
    {
        //m_validator(_pdx, _vdx);
        uint16_t cdx = make_cdx(_pdx, _vdx);
        auto it = std::lower_bound(m_params.begin(), m_params.end(), cdx);
        if (it != m_params.end() && it->cdx == cdx)
        {
            return it->view;
        }
        throw query_param_not_found("invalid parameter");
    }


    int64_t query::request(unsigned _timeout)
    {
        m_ansfer = m_request.send(_timeout, true);
        return parse_ansfer();
    }
    

    int64_t query::parse_ansfer()
    {
        int64_t value = -1;
        char const * begin = skip_spaces(m_ansfer.data());
        char const * end = m_ansfer.data() + m_ansfer.size();

        try
        {
            begin = std::from_chars(begin, end, value).ptr;
            if (static_cast<uint16_t>(value) != m_ctl_index)
            {
                assert(!(value != m_ctl_index) && "unexpected error");
                throw tas_unexpected_error("unexpected error");
            }

            begin = std::from_chars(skip_spaces(begin), end, value).ptr;
            if (value == -1)
            {
                return value;
            }

            for (auto & e : m_params)
            {
                begin = skip_spaces(begin);
                if (begin == end)
                {
                    assert(!(begin == end) && "unexpected error");
                    throw tas_unexpected_error("unexpected error");
                }
                char const * end_token = find_space_or_null(begin);
                auto v = std::string_view(begin, end_token - begin);
                e.view = (v != "err") ? v : std::string_view();
            }

            if (begin != end)
            {
                assert(!(begin != end) && "unexpected error");
                throw tas_unexpected_error("unexpected error");
            }
            return static_cast<uint64_t>(value);
        }
        catch (...)
        {
            for (auto & e : m_params)
            {
                e.view.remove_prefix(e.view.size());
            }
            throw;
        }
    }


    void query::clear_request()
    {
        m_request.clear();
        m_request.add_number(m_ctl_index);
    }
    
    
}//namespace tas
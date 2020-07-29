#include "../tas_ipc/tas_ipc.h"

#include <algorithm>
#include <cassert>
#include "controller.h"

namespace
{
    bool operator<(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return _l.pdx < _r.pdx || (_l.pdx == _r.pdx && _l.vdx < _r.vdx);
    }
    bool operator==(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return _l.pdx == _r.pdx && _l.vdx == _r.vdx;
    }
    bool operator!=(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return !(_l == _r);
    }
}//internal linkage


namespace eye
{

    controller::controller(std::string const & _ctl_name):
        m_timestamp(-1),
        m_need_rebuild(true)
    {
        m_params.reserve((TAS_MAX_REQUEST_SIZE - 12) / 5);//max count
        m_query.reset(tas_query_create_by_name(_ctl_name.c_str(), nullptr));
        if (!m_query)
        {
            assert(m_query.get() != nullptr && "invalid name");
            throw std::invalid_argument("invalid name");
        }
    }


    controller::~controller()
    {
    }


    bool controller::add(uint16_t _pdx, uint16_t _vdx)
    {
        if (m_params.size() == m_params.capacity())
        {
            return false;
        }
        auto it = std::find(m_params.begin(), m_params.end(), param_t{ _pdx, _vdx });
        if (it != m_params.end())
        {
            return false;
        }
        m_params.push_back({ _pdx, _vdx, "" });
        m_need_rebuild = true;
        return true;
    }


    bool controller::remove(uint16_t _pdx, uint16_t _vdx)
    {
        auto it = std::find(m_params.begin(), m_params.end(), param_t{ _pdx, _vdx, "" });
        if (it == m_params.end())
        {
            return false;
        }
        m_params.erase(it);
        m_need_rebuild = true;
        return false;
    }


    eUpdateResult controller::update(unsigned _timeout)
    {
        if (m_need_rebuild && !rebuild())
        {
            return eUpdateResult::Error;
        }
        int64_t timestamp = 0;
        unsigned result = tas_query_apply(m_query.get(), _timeout, &timestamp);
        if (result == TAS_ERR_SUCCESS)
        {
            m_timestamp = timestamp;
            update_values();
            return eUpdateResult::Success;
        }
        if (result == TAS_ERR_REQUEST_TIMEDOUT)
        {
            return eUpdateResult::Timedout;
        }
        return eUpdateResult::Error;
    }


    int64_t controller::timestamp() const noexcept
    {
        return m_timestamp;
    }


    std::vector<param_t> const & controller::params() const noexcept
    {
        return m_params;
    }


    std::string_view controller::get(uint16_t _pdx, uint16_t _vdx) const
    {
        auto it = std::find(m_params.begin(), m_params.end(), param_t{ _pdx, _vdx });
        if (it != m_params.end())
        {
            return it->value;
        }
        return { };
    }


    unsigned controller::max_count() const noexcept
    {
        return static_cast<unsigned>(m_params.capacity());
    }


    void controller::update_values()
    {
        for (auto & param : m_params)
        {
            char buffer[8];
            unsigned written;
            if (tas_query_get_param(m_query.get(), param.pdx, param.vdx, buffer, &written) == TAS_ERR_SUCCESS)
            {
                std::string_view sw(buffer, written);
                if (sw != param.value)
                {
                    param.value.assign(sw.data(), sw.size());
                }
            }
        }
    }


    bool controller::rebuild()
    {
        tas_query_clear(m_query.get());
        for (auto e : m_params)
        {
            if (tas_query_add_param(m_query.get(), e.pdx, e.vdx) != TAS_ERR_SUCCESS)
            {
                return false;
            }
        }
        return true;
    }


    void controller::tas_query_deleter::operator()(tas_query * _q) const noexcept
    {
        tas_query_destroy(_q);
    }


}//namespace eye



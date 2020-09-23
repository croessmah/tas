#include "../tas_ipc/tas_ipc.h"

#include <algorithm>
#include <cassert>
#include "Controller.h"

namespace
{
    [[maybe_unused]]
    bool operator<(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return _l.pdx < _r.pdx || (_l.pdx == _r.pdx && _l.vdx < _r.vdx);
    }

    [[maybe_unused]]
    bool operator==(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return _l.pdx == _r.pdx && _l.vdx == _r.vdx;
    }

    [[maybe_unused]]
    bool operator!=(::eye::param_t const & _l, ::eye::param_t const & _r) noexcept
    {
        return !(_l == _r);
    }
}//internal linkage


namespace eye
{

    Controller::Controller(std::string const & _ctl_name):
        m_name(_ctl_name),
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


    Controller::~Controller()
    {
    }


    std::string_view Controller::name() const noexcept
    {
        return m_name;
    }


    bool Controller::add(uint16_t _pdx, uint16_t _vdx)
    {
        if (m_params.size() == m_params.capacity())
        {
            return false;
        }
        if (find(_pdx, _vdx) != m_params.end())
        {
            return false;
        }
        m_params.push_back({ _pdx, _vdx, "" });
        m_need_rebuild = true;
        return true;
    }


    bool Controller::remove(uint16_t _pdx, uint16_t _vdx)
    {
        auto it = find(_pdx, _vdx);
        if (it == m_params.end())
        {
            return false;
        }
        m_params.erase(it);
        m_need_rebuild = true;
        return true;
    }


    int64_t Controller::timestamp() const noexcept
    {
        return m_timestamp;
    }


    std::vector<param_t> const & Controller::params() const noexcept
    {
        return m_params;
    }


    std::string_view Controller::get(uint16_t _pdx, uint16_t _vdx) const
    {
        auto it = find(_pdx, _vdx);
        if (it != m_params.end())
        {
            return it->value;
        }
        return { };
    }


    unsigned Controller::max_count() const noexcept
    {
        return static_cast<unsigned>(m_params.capacity());
    }


    bool Controller::update(bool & _has_update)
    {
        return update(sc_default_timeout, _has_update);
    }


    bool Controller::update(unsigned _timeout, bool & _has_update)
    {
        eUpdateResult res = update_aux(_timeout, _has_update);
        return res == eUpdateResult::Success;
    }


    eUpdateResult Controller::update_aux(unsigned _timeout, bool & _has_update)
    {
        _has_update = false;
        if (m_need_rebuild && !rebuild())
        {
            return eUpdateResult::Error;
        }
        int64_t timestamp = 0;
        unsigned result = tas_query_apply(m_query.get(), _timeout, &timestamp);
        if (result == TAS_ERR_SUCCESS)
        {
            m_timestamp = timestamp;
            _has_update = update_values();
            return eUpdateResult::Success;
        }
        if (result == TAS_ERR_REQUEST_TIMEDOUT)
        {
            return eUpdateResult::Timedout;
        }
        return eUpdateResult::Error;
    }


    bool Controller::update_values()
    {
        bool has_update = false;
        for (auto & param : m_params)
        {
            char buffer[8];
            unsigned written;
            if (tas_query_get_param(m_query.get(), param.pdx, param.vdx, buffer, &written) == TAS_ERR_SUCCESS)
            {
                std::string_view sw(buffer, written);
                if (!sw.empty() && sw != param.value)
                {
                    has_update = true;
                    param.value.assign(sw.data(), sw.size());
                }
            }
        }
        return has_update;
    }


    bool Controller::rebuild()
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


    std::vector<param_t>::iterator Controller::find(uint16_t _pdx, uint16_t _vdx) noexcept
    {
        return std::find_if(
            m_params.begin(),
            m_params.end(),
            [=](param_t const & _param)
            {
                return _param.pdx == _pdx && _param.vdx == _vdx;
            }
        );
    }


    std::vector<param_t>::const_iterator Controller::find(uint16_t _pdx, uint16_t _vdx) const noexcept
    {
        auto & ctl = const_cast<Controller &>(*this);
        return ctl.find(_pdx, _vdx);
    }


    void Controller::tas_query_deleter::operator()(tas_query * _q) const noexcept
    {
        tas_query_destroy(_q);
    }


}//namespace eye



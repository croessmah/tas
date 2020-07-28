#include "../tas_ipc/tas_ipc.h"

#include <algorithm>
#include "single_request.h"

namespace eye
{


    single_request::single_request(std::string _ctl_name) :
        m_controller(std::move(_ctl_name)),
        m_timestamp(-1),
        m_need_rebuild(true)
    {
        m_idxs.reserve((TAS_MAX_REQUEST_SIZE - 12) / 5);

        m_query.reset(tas_query_create_by_name(m_controller.c_str(), nullptr));
        if (!m_query)
        {
            //todo: throw
        }
    }


    single_request::~single_request()
    {
    }


    bool single_request::add(uint16_t _pdx, uint16_t _vdx)
    {
        if (m_idxs.size() == m_idxs.capacity())
        {
            return false;
        }
        auto it = std::find(m_idxs.begin(), m_idxs.end(), param_t{ _pdx, _vdx });
        if (it != m_idxs.end())
        {
            return false;
        }
        m_idxs.push_back({ _pdx, _vdx });
        m_need_rebuild = true;
        return true;
    }


    bool single_request::remove(uint16_t _pdx, uint16_t _vdx)
    {
        auto it = std::find(m_idxs.begin(), m_idxs.end(), param_t{ _pdx, _vdx });
        if (it == m_idxs.end())
        {
            return false;
        }
        m_idxs.erase(it);
        m_need_rebuild = true;
        return false;
    }

    int single_request::send(unsigned _timeout)
    {
        if (m_need_rebuild && !rebuild())
        {
            return -1;
        }
        int64_t timestamp = 0;
        unsigned result = tas_query_apply(m_query.get(), _timeout, &timestamp);
        if (result == TAS_ERR_SUCCESS)
        {
            m_timestamp = timestamp;
            return 0;
        }
        if (result == TAS_ERR_REQUEST_TIMEDOUT)
        {
            return 1;
        }
        return -1;
    }


    int64_t single_request::timestamp() const noexcept
    {
        return m_timestamp;
    }


    std::vector<param_t> const & single_request::params() const noexcept
    {
        return m_idxs;
    }

    bool single_request::fill(std::vector<param_with_value_t>& _result) const
    {
        if (m_need_rebuild)
        {
            return false;
        }
        _result.clear();
        char buffer[8];
        unsigned written = 0;
        for (auto e : m_idxs)
        {
            tas_query_get_param(m_query.get(), e.pdx, e.vdx, buffer, &written);
            if (written != 0)
            {
                _result.push_back({ e, {buffer, written} });
            }
        }
        return true;
    }

    bool single_request::rebuild()
    {
        tas_query_clear(m_query.get());
        for (auto e : m_idxs)
        {
            if (tas_query_add_param(m_query.get(), e.pdx, e.vdx) != TAS_ERR_SUCCESS)
            {
                return false;
            }
        }
        return true;
    }


    void single_request::tas_query_deleter::operator()(tas_query * _q) const noexcept
    {
        tas_query_destroy(_q);
    }

}//namespace eye



#pragma once
#ifndef INCLUDE_OPERATOR_EYE_CONTROLLER_H
#define INCLUDE_OPERATOR_EYE_CONTROLLER_H
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

struct tas_query;

namespace eye
{

    enum class eUpdateResult
    {
        Error = -1,
        Success = 0,
        Timedout = 1
    };


    struct param_t
    {
        uint16_t pdx;
        uint16_t vdx;
        std::string value;
    };
    

    class controller
    {
    public:
        controller(std::string const & _ctl_name);
        ~controller();
        std::string_view name() const noexcept;
        bool add(uint16_t _pdx, uint16_t _vdx);
        bool remove(uint16_t _pdx, uint16_t _vdx);
        eUpdateResult update(unsigned _timeout);
        eUpdateResult update(unsigned _timeout, bool & _has_update);
        int64_t timestamp() const noexcept;
        std::vector<param_t> const & params() const noexcept;        
        std::string_view get(uint16_t _pdx, uint16_t _vdx) const;
        unsigned max_count() const noexcept;

        int compare(controller const & _s) const
        {
            return m_name.compare(_s.name());
        }

        int compare(std::string_view _s) const
        {
            return m_name.compare(_s);
        }
    private:
        bool update_values();
        bool rebuild();
        struct tas_query_deleter
        {
            void operator()(tas_query * _q) const noexcept;
        };
        std::vector<param_t> m_params;
        std::string m_name;
        int64_t m_timestamp;
        std::unique_ptr<tas_query, tas_query_deleter> m_query;
        bool m_need_rebuild;
    };


    inline bool operator<(controller const & _f, controller const & _s) noexcept
    {
        return _f.compare(_s) < 0;
    }


    inline bool operator==(controller const & _f, controller const & _s) noexcept
    {
        return _f.compare(_s) == 0;
    }


    inline bool operator<(controller const & _f, std::string_view _s) noexcept
    {
        return _f.compare(_s) < 0;
    }


    inline bool operator<(std::string_view _f, controller const & _s) noexcept
    {
        return _s.compare(_f) > 0;
    }


    inline bool operator==(controller const & _f, std::string_view _s) noexcept
    {
        return _f.compare(_s) == 0;
    }


    inline bool operator==(std::string_view _f, controller const & _s) noexcept
    {
        return _s.compare(_f) == 0;
    }
}//namespace eye

#endif //INCLUDE_OPERATOR_EYE_CONTROLLER_H
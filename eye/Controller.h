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


    class Controller
    {
    public:
        Controller(Controller const &) = delete;
        Controller & operator=(Controller const &) = delete;
        Controller(std::string const & _ctl_name);
        ~Controller();
        std::string_view name() const noexcept;
        bool add(uint16_t _pdx, uint16_t _vdx);
        bool remove(uint16_t _pdx, uint16_t _vdx);
        int64_t timestamp() const noexcept;
        std::vector<param_t> const & params() const noexcept;
        std::string_view get(uint16_t _pdx, uint16_t _vdx) const;
        unsigned max_count() const noexcept;

        int compare(Controller const & _s) const
        {
            return m_name.compare(_s.name());
        }

        int compare(std::string_view _s) const
        {
            return m_name.compare(_s);
        }

        bool update(bool & _has_update);//default timeout
        bool update(unsigned _timeout, bool & _has_update);
    private:
        static constexpr unsigned sc_default_timeout = 500;
        eUpdateResult update_aux(unsigned _timeout, bool & _has_update);
        bool update_values();
        bool rebuild();
        std::vector<param_t>::iterator find(uint16_t _pdx, uint16_t _vdx) noexcept;
        std::vector<param_t>::const_iterator find(uint16_t _pdx, uint16_t _vdx) const noexcept;
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


    inline bool operator<(Controller const & _f, Controller const & _s) noexcept
    {
        return _f.compare(_s) < 0;
    }


    inline bool operator==(Controller const & _f, Controller const & _s) noexcept
    {
        return _f.compare(_s) == 0;
    }


}//namespace eye

#endif //INCLUDE_OPERATOR_EYE_CONTROLLER_H

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
        bool add(uint16_t _pdx, uint16_t _vdx);
        bool remove(uint16_t _pdx, uint16_t _vdx);
        eUpdateResult update(unsigned _timeout);
        int64_t timestamp() const noexcept;
        std::vector<param_t> const & params() const noexcept;        
        std::string_view get(uint16_t _pdx, uint16_t _vdx) const;
        unsigned max_count() const noexcept;
    private:
        void update_values();
        bool rebuild();
        struct tas_query_deleter
        {
            void operator()(tas_query * _q) const noexcept;
        };
        std::vector<param_t> m_params;
        int64_t m_timestamp;
        std::unique_ptr<tas_query, tas_query_deleter> m_query;
        bool m_need_rebuild;
    };


}//namespace eye

#endif //INCLUDE_OPERATOR_EYE_CONTROLLER_H
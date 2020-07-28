#pragma once
#ifndef INCLUDE_OPERATOR_EYE_SINGLE_REQUEST_H
#define INCLUDE_OPERATOR_EYE_SINGLE_REQUEST_H
#include <vector>
#include <string>
#include <cstdint>

struct tas_query;

namespace eye
{


    struct param_t
    {
        uint16_t pdx;
        uint16_t vdx;
        friend bool operator<(param_t _l, param_t _r) noexcept
        {
            return _l.pdx < _r.pdx || (_l.pdx == _r.pdx && _l.vdx < _r.vdx);
        }
        friend bool operator==(param_t _l, param_t _r) noexcept
        {
            return _l.pdx == _r.pdx && _l.vdx == _r.vdx;
        }
        friend bool operator!=(param_t _l, param_t _r) noexcept
        {
            return !(_l == _r);
        }
    };


    struct param_with_value_t
    {
        param_t param;
        std::string value;
    };


    class single_request
    {
    public:
        single_request(std::string _ctl_name);
        ~single_request();
        bool add(uint16_t _pdx, uint16_t _vdx);
        bool remove(uint16_t _pdx, uint16_t _vdx);
        int send(unsigned _timeout);
        int64_t timestamp() const noexcept;
        std::vector<param_t> const & params() const noexcept;
        bool fill(std::vector<param_with_value_t> & _result) const;
        std::string get(uint16_t _pdx, uint16_t _vdx) const;
    private:
        bool rebuild();
        struct tas_query_deleter
        {
            void operator()(tas_query * _q) const noexcept;
        };
        std::string m_controller;
        std::vector<param_t> m_idxs;
        int64_t m_timestamp;
        std::unique_ptr<tas_query, tas_query_deleter> m_query;
        bool m_need_rebuild;
    };


}//namespace eye

#endif //INCLUDE_OPERATOR_EYE_SINGLE_REQUEST_H
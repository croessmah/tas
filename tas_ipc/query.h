#pragma once
#ifndef INCLUDE_TAS_IPC_QUERY_H
#define INCLUDE_TAS_IPC_QUERY_H
#include <cstdint>
#include <vector>
#include <string>
#include <cstdint>
#include <string_view>
#include "request.h"

struct tas_query {};

namespace tas
{
    class query: 
        public tas_query
    {
        query(query const &) = delete;
        query & operator=(query const &) = delete;
    public:
        static constexpr unsigned sc_infinite = ::tas::request::sc_infinite;
        query(char const * _ctl_name);
        ~query();
        bool add_param(uint16_t _pdx, uint16_t _vdx);
        std::string_view get_param(uint16_t _pdx, uint16_t _vdx);
        int64_t request(unsigned _timeout);
        void clear() noexcept;
    private:
        int64_t parse_ansfer();
        void clear_request();
        struct param;
        std::vector<param> m_params;
        ::tas::request m_request;
        std::string m_ansfer;
        uint16_t m_ctl_index;
    };


    TAS_INTRODUCE_GENERAL_EXCEPTION(query_invalid_ctl, TAS_ERR_INVALID_CTL);
    TAS_INTRODUCE_GENERAL_EXCEPTION(query_param_not_found, TAS_ERR_PARAM_NOT_FOUND);
    TAS_INTRODUCE_GENERAL_EXCEPTION(tas_unexpected_error, TAS_ERR_UNEXPECTED);
}//namespace tas



#endif //INCLUDE_TAS_IPC_QUERY_H
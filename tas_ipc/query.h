#pragma once
#ifndef INCLUDE_TAS_IPC_QUERY_H
#define INCLUDE_TAS_IPC_QUERY_H
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

struct tas_query {};

namespace tas
{
    class query: 
        public tas_query
    {
    public:
        void add_rapam(uint16_t _pdx, uint16_t _vdx);
        void remove_param(uint16_t _pdx, uint16_t _vdx);
        std::string_view get_param(uint16_t _pdx, uint16_t _vdx);
    private:
        struct param
        {
            std::string_view view;
            uint16_t cdx;
        };

        std::vector<uint16_t> m_cdxs;
        std::vector<param> m_params;
        std::vector<std::string> m_query_texts;
        std::vector<std::string> m_ansfer_texts;
        uint16_t m_ctl_index;
    };
}//namespace tas



#endif //INCLUDE_TAS_IPC_QUERY_H
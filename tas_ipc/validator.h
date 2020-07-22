#pragma once
#ifndef INCLUDE_TAS_IPC_VALIDATOR_H
#define INCLUDE_TAS_IPC_VALIDATOR_H
#include <cstdint>

namespace tas
{

    class vdx_validator
    {
    public:
        constexpr vdx_validator(uint16_t _vdxs_count):
            m_vdxs_count(_vdxs_count)
        {}

        void operator()(uint16_t _vdx) const
        {
            if (m_vdxs_count <= _vdx)
            {
                //todo: throw invalid vdx
            }
        }
    private:
        const uint16_t m_vdxs_count;
    };

    template<uint16_t PdxsCount>
    class validator
    {
    private:
    public:
        void operator()(uint16_t _pdx, uint16_t _vdx) const
        {
            if (PdxsCount <= _pdx)
            {
                //todo: throw invalid pdx
            }                 
        }
        const vdx_validator m_vdxs[PdxsCount];
    };


}//namespace tas


#endif //INCLUDE_TAS_IPC_VALIDATOR_H
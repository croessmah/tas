#include "query.h"


namespace
{
    inline uint16_t make_cdx(uint16_t _pdx, uint16_t _vdx) noexcept
    {
        return (_pdx << uint16_t(10u)) | (_vdx & uint16_t(0x3FF));
    }
}//internal linkage




namespace tas
{



}//namespace tas
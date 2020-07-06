#include <cstring>
#include <cassert>
#include "controller.h"

tas_controller::tas_controller(char const * _name, uint32_t _ip) noexcept:
    m_ip(_ip)
{
    assert(strlen(_name) < sizeof(m_name) && "name buffer is too small");
    strcpy(m_name, _name);
}

uint32_t tas_controller::ip() noexcept
{
    return m_ip;
}

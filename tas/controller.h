#pragma once
#ifndef INCLUDE_TAS_CONTROLLER_H
#define INCLUDE_TAS_CONTROLLER_H

#include <cstdint>

class tas_controller
{
public:
    tas_controller(char const * _name, uint32_t _ip) noexcept;
    uint32_t ip() noexcept;
private:    
    char m_name[8];
    uint32_t m_ip;
};




#endif //INCLUDE_TAS_CONTROLLER_H
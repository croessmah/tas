// tas.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <WinSock2.h>
#include <Windows.h>
#include <netioapi.h>
#include <Iphlpapi.h>
#include <ws2tcpip.h>
#include <iostream>
#include "application.h"
#include "controller.h"
#include "module_timed_stop.h"
#include "module_sniffer.h"

#include <iostream>

bool str_to_ip(char const * _str, uint32_t & _ip) noexcept
{
    IN_ADDR addr;
    if (inet_pton(AF_INET, _str, &addr) == 1) {
        _ip = static_cast<uint32_t>(addr.S_un.S_addr);
        return true;
    }
    return false;
}


char * ip_to_str(uint32_t _ip, char * _buf, unsigned _size) noexcept
{
    IN_ADDR addr;
    addr.S_un.S_addr = _ip;
    inet_ntop(AF_INET, &addr, _buf, _size);
    return _buf;
}


int start_application()
{
    uint32_t srgm_ip = 0; ;
    uint32_t ft5p_ip = 0; ;
    if (!str_to_ip("192.168.0.112", srgm_ip) || !str_to_ip("192.168.0.111", ft5p_ip))
    {
        std::cout << "convert ip error" << std::endl;
        return EXIT_SUCCESS;
    }
    tas_controller controllers[2] =
    {
        {"srgm", srgm_ip},
        {"ft5p", ft5p_ip},
    };

    tas_application app(300 /*event wait ms period*/);
    tas_md_sniffer md_sniff(app, controllers, 2);
    tas_md_timed_stop mts{ 120000/*app stop timeout ms*/ };

    if (!app.run(mts, md_sniff))
    {
        std::cout << "error: " << app.error() << std::endl;
    }
    else
    {
        std::cout << "success" << std::endl;
    }
    std::cout << sizeof(long double) << std::endl;

    return 0;
}


int main()
{
    return start_application();
}


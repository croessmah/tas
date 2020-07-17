// tas.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <WinSock2.h>
//#include <Windows.h>
//#include <netioapi.h>
//#include <Iphlpapi.h>
#include <ws2tcpip.h>
#include <cassert>
#include "application.h"
#include "controller.h"
#include "module_timed_stop.h"
#include "module_sniffer.h"
#include "module_ipc.h"


uint32_t str_to_ip(char const * _str) noexcept
{
    IN_ADDR addr;
    bool ip_cvt_result = inet_pton(AF_INET, _str, &addr) == 1;
    assert(ip_cvt_result && "invalid ip");
    return static_cast<uint32_t>(addr.S_un.S_addr);
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
    tas_controller controllers[2] =
    {
        {str_to_ip("192.168.0.112")}, //srgm
        {str_to_ip("192.168.0.111")}, //ft5p
    };
    
    tas_application app(100 /*event wait ms period*/);
    tas_md_sniffer md_sniff(app, controllers, 2);
    //tas_md_timed_stop mts{ 1200000/*app stop timeout ms*/ };
    tas_md_ipc mipc(controllers, 2, 3000);
    
    if (!app.run(/*mts, */md_sniff, mipc))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int main()
{
    return start_application();
}


#include <Windows.h>
#include <cstring>
#include "app_service.h"
#include "app_console.h"
#include "srv_utils.h"

int wmain(int argc, wchar_t ** argv)
{
    if (argc < 2)
    {
        tas_app_service srv(100);
        return srv.run();
    }    

    if (argc == 0)
    {
        return EXIT_FAILURE;
    }

    if (wcscmp(argv[1], L"app") == 0)
    {
        tas_app_console app(100);
        return app.run();
    }
        
    bool(*utils_function)(wchar_t const *) = nullptr;
    if (wcscmp(argv[1], L"install") == 0)
    {
        utils_function = tas_srv_utils_install_service;
    }
    else if (wcscmp(argv[1], L"remove") == 0)
    {
        utils_function = tas_srv_utils_remove_service;
    }
    else if (wcscmp(argv[1], L"start") == 0)
    {
        utils_function = tas_srv_utils_start_service;
    }
    else if (wcscmp(argv[1], L"stop") == 0)
    {
        utils_function = tas_srv_utils_stop_service;
    }
    return (utils_function && utils_function(tas_app_service::service_name())) ? 
        EXIT_SUCCESS : 
        EXIT_FAILURE ;
}

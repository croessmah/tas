#include <Windows.h>
#include <cstring>
#include <cstdlib>
#include "app_service.h"
#include "app_console.h"
#include "srv_utils.h"



#ifdef NDEBUG //no debug
#define APP_TYPE tas_app_service
#else //debug
#define APP_TYPE tas_app_console
#endif 



int start(int argc, wchar_t ** argv)
{
    if (argc < 2)
    {
        APP_TYPE app(100);
        return app.run();
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


int main()
{
    LPWSTR * szArglist;
    int args_count = 0;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &args_count);
    if( NULL == szArglist )
    {
       return EXIT_FAILURE;
    }

    int result = start(args_count, szArglist);
    LocalFree(szArglist);
    return result;
}



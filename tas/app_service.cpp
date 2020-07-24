#include <Windows.h>
#include "application.h"
#include "controller.h"
#include "module_sniffer.h"
#include "module_ipc.h"
#include "app_service.h"


namespace
{
    wchar_t g_service_name[] = L"tas";
    SERVICE_STATUS g_service_status;
    SERVICE_STATUS_HANDLE g_service_status_handle;
    unsigned g_tick_time = 100;


    DWORD WINAPI srv_control_handler(DWORD _code, DWORD, LPVOID, LPVOID _context)
    {
        DWORD result = NO_ERROR;
        switch (_code)
        {
        case SERVICE_CONTROL_STOP:
            //[[fallthrough]]
        case SERVICE_CONTROL_SHUTDOWN:
            if (g_service_status.dwCurrentState != SERVICE_STOP &&
                g_service_status.dwCurrentState != SERVICE_STOP_PENDING)
            {
                g_service_status.dwWin32ExitCode = 0;
                g_service_status.dwCurrentState = SERVICE_STOP_PENDING;
                SetServiceStatus(g_service_status_handle, &g_service_status);
                static_cast<tas_application *>(_context)->stop(0);
            }
        case SERVICE_CONTROL_INTERROGATE:
            break;
        default:
            result = ERROR_CALL_NOT_IMPLEMENTED;
            break;
        }
        return result;
    }


    bool srv_init(tas_application & _app) noexcept
    {
        g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        g_service_status.dwCurrentState = SERVICE_START_PENDING;
        g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        g_service_status.dwWin32ExitCode = 0;
        g_service_status.dwServiceSpecificExitCode = 0;
        g_service_status.dwCheckPoint = 0;
        g_service_status.dwWaitHint = 0;
        g_service_status_handle = RegisterServiceCtrlHandlerExW(g_service_name, srv_control_handler, &_app);
        return g_service_status_handle != nullptr;
    }
    

    void WINAPI service_main(DWORD, LPWSTR*)
    {
        tas_controller controllers[2] = {
            "192.168.0.112",
            "192.168.0.111"
        };
        tas_application app(g_tick_time);
        tas_md_sniffer sniffer(app, controllers, 2);
        tas_md_ipc ipc(controllers, 2, 2000);

        if (!srv_init(app))
        {
            return;
        }
        g_service_status.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(g_service_status_handle, &g_service_status);
        app.run(sniffer, ipc);
        if (g_service_status.dwCurrentState != SERVICE_STOPPED)
        {
            g_service_status.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(g_service_status_handle, &g_service_status);
        }
    }
}//internal linkage



tas_app_service::tas_app_service(unsigned _tick_time) noexcept
{
    g_tick_time = _tick_time;
}


int tas_app_service::run() noexcept
{
    SERVICE_TABLE_ENTRYW table[2] =
    {
        {g_service_name, service_main},
        {nullptr, nullptr}
    };
    if (!StartServiceCtrlDispatcherW(table))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

wchar_t const * tas_app_service::service_name() noexcept
{
    return g_service_name;
}






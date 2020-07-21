#include <ws2tcpip.h>
#include <Windows.h>
#include <cassert>
#include "application.h"
#include "controller.h"
#include "module_sniffer.h"
#include "module_ipc.h"




namespace
{
    SERVICE_STATUS gc_service_status;
    SERVICE_STATUS_HANDLE gc_service_status_handle;
    wchar_t gc_service_name[] = L"tas";

    uint32_t str_to_ip(char const * _str) noexcept
    {
        IN_ADDR addr;
        bool ip_cvt_result = (inet_pton(AF_INET, _str, &addr) == 1);
        (void)ip_cvt_result;
        assert(ip_cvt_result && "invalid ip");
        return static_cast<uint32_t>(addr.S_un.S_addr);
    }



    bool utils_open_service_all_access(wchar_t const * _name, DWORD _mode, SC_HANDLE & _manager, SC_HANDLE & _service)
    {
        SC_HANDLE scmanager = OpenSCManager(
            NULL,
            NULL,
            SC_MANAGER_ALL_ACCESS
        );
        if (!scmanager)
        {
            return false;
        }

        SC_HANDLE service = OpenService(
            scmanager,
            _name,
            _mode);

        if (!service)
        {
            CloseServiceHandle(scmanager);
            return false;
        }
        _manager = scmanager;
        _service = service;
        return true;
    }


    bool utils_install_service(wchar_t const * _name)
    {
        unsigned constexpr lc_max_path = 1024;
        wchar_t path_buffer[lc_max_path] = {};

        DWORD count = GetModuleFileName(NULL, path_buffer, lc_max_path);
        DWORD err = GetLastError();
        if (count == 0 || (count == lc_max_path && err == ERROR_INSUFFICIENT_BUFFER))
        {
            return false;
        }

        SC_HANDLE scmanager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (!scmanager) {
            return false;
        }
        
        SC_HANDLE service = CreateServiceW(
            scmanager,
            _name,
            _name,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            path_buffer,
            NULL, NULL, NULL, NULL, NULL
        );
        if (!service)
        {
            CloseServiceHandle(scmanager);
            return false;
        }
        CloseServiceHandle(service);
        CloseServiceHandle(scmanager);        
        return true;
    }


    bool utils_remove_service(wchar_t const * _name)
    {
        SC_HANDLE scmanager = nullptr;
        SC_HANDLE service = nullptr;
        
        if (!utils_open_service_all_access(_name, DELETE, scmanager, service))
        {
            return false;
        }
        bool result = DeleteService(service);
        CloseServiceHandle(service);
        CloseServiceHandle(scmanager);
        return result;
    }


    bool utils_start_service(wchar_t const * _name)
    {
        SC_HANDLE scmanager = nullptr;
        SC_HANDLE service = nullptr;

        if (!utils_open_service_all_access(_name, SERVICE_START, scmanager, service))
        {
            return false;
        }
        
        bool result = StartServiceW(service, 0, NULL);
        CloseServiceHandle(service);
        CloseServiceHandle(scmanager);
        return result;
    }


    bool utils_stop_service(wchar_t const * _name)
    {
        SC_HANDLE scmanager = nullptr;
        SC_HANDLE service = nullptr;

        if (!utils_open_service_all_access(_name, SERVICE_STOP, scmanager, service))
        {
            return false;
        }

        SERVICE_STATUS status;
        bool result = ControlService(service, SERVICE_CONTROL_STOP, &status);
        CloseServiceHandle(service);
        CloseServiceHandle(scmanager);
        return result;
    }
}//internall inkage



DWORD WINAPI srv_control_handler(DWORD _code, DWORD, LPVOID, LPVOID _context)
{
    DWORD result = NO_ERROR;
    switch (_code)
    {
    case SERVICE_CONTROL_STOP:
        //[[fallthrough]]
    case SERVICE_CONTROL_SHUTDOWN:
        if (gc_service_status.dwCurrentState != SERVICE_STOP &&
            gc_service_status.dwCurrentState != SERVICE_STOP_PENDING)
        {
            gc_service_status.dwWin32ExitCode = 0;
            gc_service_status.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(gc_service_status_handle, &gc_service_status);
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


bool srv_init_status(wchar_t const * const _name, tas_application & _app)
{
    gc_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gc_service_status.dwCurrentState = SERVICE_START_PENDING;
    gc_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    gc_service_status.dwWin32ExitCode = 0;
    gc_service_status.dwServiceSpecificExitCode = 0;
    gc_service_status.dwCheckPoint = 0;
    gc_service_status.dwWaitHint = 0;
    gc_service_status_handle = RegisterServiceCtrlHandlerExW(_name, srv_control_handler, &_app);
    return gc_service_status_handle != nullptr;
}



void WINAPI service_main(DWORD, LPWSTR*)
{
    tas_controller controllers[2] =
    {
        {str_to_ip("192.168.0.112")}, //srgm
        {str_to_ip("192.168.0.111")}, //ft5p
    };

    tas_application app(100 /*event wait ms period*/);

    if (!srv_init_status(gc_service_name, app))
    {
        return;
    }
       
    tas_md_sniffer md_sniff(app, controllers, 2);
    tas_md_ipc mipc(controllers, 2, 3000);

    gc_service_status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(gc_service_status_handle, &gc_service_status);
    
    app.run(/*mts, */md_sniff, mipc);
    if (gc_service_status.dwCurrentState != SERVICE_STOPPED)
    {
        gc_service_status.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(gc_service_status_handle, &gc_service_status);
    }    
}



int wmain(int argc, wchar_t ** argv)
{
    if (argc > 1)
    {
        bool(*utils_function)(wchar_t const *) = nullptr;
        if (wcscmp(argv[1], L"install") == 0)
        {
            utils_function = utils_install_service;
        }
        else if (wcscmp(argv[1], L"remove") == 0)
        {
            utils_function = utils_remove_service;
        }
        else if (wcscmp(argv[1], L"start") == 0)
        {
            utils_function = utils_start_service;
        }
        else if (wcscmp(argv[1], L"stop") == 0)
        {
            utils_function = utils_stop_service;
        }
        return (utils_function && utils_function(gc_service_name)) ? 
            EXIT_SUCCESS : 
            EXIT_FAILURE;
    }
       
    SERVICE_TABLE_ENTRYW table[2] = 
    {
        {gc_service_name, service_main},
        {nullptr, nullptr}
    };

    if (!StartServiceCtrlDispatcherW(table))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <Windows.h>
#include "srv_utils.h"


namespace
{
    bool tas_srv_utils_open_service_all_access(wchar_t const * _name, DWORD _mode, SC_HANDLE & _manager, SC_HANDLE & _service)
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
}//internal linkage


bool tas_srv_utils_install_service(wchar_t const * _name)
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


bool tas_srv_utils_remove_service(wchar_t const * _name)
{
    SC_HANDLE scmanager = nullptr;
    SC_HANDLE service = nullptr;

    if (!tas_srv_utils_open_service_all_access(_name, DELETE, scmanager, service))
    {
        return false;
    }
    bool result = DeleteService(service);
    CloseServiceHandle(service);
    CloseServiceHandle(scmanager);
    return result;
}


bool tas_srv_utils_start_service(wchar_t const * _name)
{
    SC_HANDLE scmanager = nullptr;
    SC_HANDLE service = nullptr;

    if (!tas_srv_utils_open_service_all_access(_name, SERVICE_START, scmanager, service))
    {
        return false;
    }

    bool result = StartServiceW(service, 0, NULL);
    CloseServiceHandle(service);
    CloseServiceHandle(scmanager);
    return result;
}


bool tas_srv_utils_stop_service(wchar_t const * _name)
{
    SC_HANDLE scmanager = nullptr;
    SC_HANDLE service = nullptr;

    if (!tas_srv_utils_open_service_all_access(_name, SERVICE_STOP, scmanager, service))
    {
        return false;
    }

    SERVICE_STATUS status;
    bool result = ControlService(service, SERVICE_CONTROL_STOP, &status);
    CloseServiceHandle(service);
    CloseServiceHandle(scmanager);
    return result;
}
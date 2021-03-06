#pragma once
#ifndef INCLUDE_TAS_SRV_UTILS_H
#define INCLUDE_TAS_SRV_UTILS_H


bool tas_srv_utils_install_service(wchar_t const * _name);
bool tas_srv_utils_remove_service(wchar_t const * _name);
bool tas_srv_utils_start_service(wchar_t const * _name);
bool tas_srv_utils_stop_service(wchar_t const * _name);


#endif //INCLUDE_TAS_SRV_UTILS_H
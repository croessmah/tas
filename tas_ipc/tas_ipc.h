#pragma once
#ifndef INCLUDE_TAS_IPC_TAS_IPC_H
#define INCLUDE_TAS_IPC_TAS_IPC_H
#include <stdint.h>

#ifdef TAS_IPC_BUILD_DLL
#define TAS_IPC_API __declspec(dllexport)
#else
#define TAS_IPC_API __declspec(dllimport)
#endif


#ifdef __cplusplus
#define TAS_IPC_NOEXCEPT noexcept
extern "C" {
#else 
#define TAS_IPC_NOEXCEPT 
#endif 

#define TAS_ERR_INVALID_SIZE (~0u)
#define TAS_ERR_SERVICE_NOT_FOUND (TAS_INVALID_SIZE - 1)
#define TAS_ERR_SERVICE_CONNECTION_ERROR (TAS_INVALID_SIZE - 2)
#define TAS_ERR_SERVICE_BUSY (TAS_INVALID_SIZE - 3)
#define TAS_ERR_OVERFLOW (TAS_INVALID_SIZE - 4)
#define TAS_ERR_BAD_ALLOCATION (TAS_INVALID_SIZE - 5)
#define TAS_ERR_INVALID_VDX (TAS_INVALID_SIZE - 6)
#define TAS_ERR_INVALID_PDX (TAS_INVALID_SIZE - 7)

typedef struct tas_query * tas_query_handle;

TAS_IPC_API unsigned tas_query_run_service() TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_wait_service(unsigned _timeout) TAS_IPC_NOEXCEPT;

TAS_IPC_API tas_query_handle tas_query_create_by_name(char const * _ctl_name) TAS_IPC_NOEXCEPT;
TAS_IPC_API tas_query_handle tas_query_create(unsigned _ctl_index) TAS_IPC_NOEXCEPT;
TAS_IPC_API void tas_query_destroy(tas_query_handle _query) TAS_IPC_NOEXCEPT;

TAS_IPC_API unsigned tas_query_add_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_remove_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_apply(tas_query_handle _query, unsigned long _timeout) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_get_rapam(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx, char * _result_bufer, unsigned _result_size) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_get_last_update_ctl(tas_query_handle _query) TAS_IPC_NOEXCEPT;

#ifdef __cplusplus
}
#endif 


#endif //INCLUDE_TAS_IPC_TAS_IPC_H
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



#define TAS_ERR_BEGIN_UCODE (~0u)
#define TAS_ERR_MAKE_UCODE(code) (TAS_ERR_BEGIN_UCODE - (code))

#define TAS_ERR_SUCCESS                         (0u)
#define TAS_ERR_INVALID_SIZE                    TAS_ERR_MAKE_UCODE(0)
#define TAS_ERR_REQUEST_OVERFLOW                TAS_ERR_MAKE_UCODE(1)
#define TAS_ERR_REQUEST_TIMEDOUT                TAS_ERR_MAKE_UCODE(2)
#define TAS_ERR_REQUEST_INVALID                 TAS_ERR_MAKE_UCODE(3)
#define TAS_ERR_SERVICE_BUSY                    TAS_ERR_MAKE_UCODE(4)
#define TAS_ERR_CONNECTION_ERROR                TAS_ERR_MAKE_UCODE(5)
#define TAS_ERR_INVALID_CTL                     TAS_ERR_MAKE_UCODE(6)
#define TAS_ERR_PARAM_NOT_FOUND                 TAS_ERR_MAKE_UCODE(7)
#define TAS_ERR_UNEXPECTED                      TAS_ERR_MAKE_UCODE(8)
#define TAS_ERR_BAD_ALLOCATION                  TAS_ERR_MAKE_UCODE(9)
#define TAS_ERR_INVALID_PARAMETER               TAS_ERR_MAKE_UCODE(10)
#define TAS_ERR_ALREADY_CONTAINED               TAS_ERR_MAKE_UCODE(11)
#define TAS_ERR_VALUE_NOT_FOUND                 TAS_ERR_MAKE_UCODE(12)
/*
#define TAS_ERR_INVALID_VDX                     TAS_ERR_MAKE_UCODE(7)
#define TAS_ERR_INVALID_PDX                     TAS_ERR_MAKE_UCODE(8)
*/

typedef struct tas_query * tas_query_handle;

//TAS_IPC_API unsigned tas_query_run_service() TAS_IPC_NOEXCEPT;
//TAS_IPC_API unsigned tas_query_wait_service(unsigned _timeout) TAS_IPC_NOEXCEPT;

TAS_IPC_API tas_query_handle tas_query_create_by_name(char const * _ctl_name, unsigned * _e) TAS_IPC_NOEXCEPT;
TAS_IPC_API void tas_query_destroy(tas_query_handle _query) TAS_IPC_NOEXCEPT;

TAS_IPC_API unsigned tas_query_add_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_apply(tas_query_handle _query, unsigned _timeout, int64_t * _update_lost) TAS_IPC_NOEXCEPT;
TAS_IPC_API unsigned tas_query_get_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx, char * _result, unsigned * _written) TAS_IPC_NOEXCEPT;

#ifdef __cplusplus
}
#endif 


#endif //INCLUDE_TAS_IPC_TAS_IPC_H
#pragma once
#ifndef INCLUDE_TAS_TYPES_H
#define INCLUDE_TAS_TYPES_H


typedef unsigned long tas_size;    
typedef unsigned long tas_error;
typedef void * tas_handle;
class tas_overlapped_server;
class tas_named_pipe;
struct tas_pipe_options;
struct tas_operation;

typedef void(*tas_operation_callback)(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);


#endif //INCLUDE_TAS_TYPES_H
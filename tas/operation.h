#pragma once
#ifndef INCLUDE_TAS_OPERATION_H
#define INCLUDE_TAS_OPERATION_H

#include "types.h"

struct tas_operation
{
    tas_operation_callback callback;
    tas_handle handle;
    void * exdata;
    char * buffer;    
    tas_size size;
    tas_size written;
};


#endif //INCLUDE_TAS_OPERATION_H
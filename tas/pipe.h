#pragma once
#ifndef INCLUDE_TAS_PIPE_H
#define INCLUDE_TAS_PIPE_H

#include "types.h"


enum eTAS_NPIPE_MODE
{
    TAS_NP_MODE_IN                  = 1 << 0,
    TAS_NP_MODE_OUT                 = 1 << 1,
    TAS_NP_MODE_INOUT               = TAS_NP_MODE_IN | TAS_NP_MODE_OUT,
    TAS_NP_MODE_WMSG                = 1 << 2,
    TAS_NP_MODE_RMSG                = 1 << 3,
    TAS_NP_MODE_ALLOW_REMOTE        = 1 << 4
};


struct tas_pipe_options
{
    tas_size max_instances;
    tas_size sys_in_buffer_size;
    tas_size sys_out_buffer_size;
    unsigned mode;
};

class tas_named_pipe
{
    tas_named_pipe(tas_named_pipe const &) = delete;
    tas_named_pipe & operator=(tas_named_pipe const &) = delete;
public:
    tas_named_pipe(tas_named_pipe && _src) noexcept;
    tas_named_pipe & operator=(tas_named_pipe && _src) noexcept;

    tas_named_pipe();
    ~tas_named_pipe();

    //todo: tas_named_pipe new_instance(tas_error & _e) noexcept;
    tas_error create(wchar_t const * _name, tas_pipe_options const & _options) noexcept;
    tas_error disconnect() noexcept;
    void close() noexcept;
    bool valid() noexcept;
    tas_handle handle() noexcept;
private:
    tas_handle m_handle;
    tas_pipe_options m_options;
};


#endif //INCLUDE_TAS_PIPE_H
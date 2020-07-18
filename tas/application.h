#pragma once
#ifndef INCLUDE_TAS_APPLICATION_H
#define INCLUDE_TAS_APPLICATION_H

#include "types.h"
#include "overlapped_server.h"
#include "pipe.h"


enum eTAS_APP_ERROR
{
    TAS_AER_NOT_READY,

};


class tas_application
{
public:
    static constexpr tas_size sc_srgm_index = 0;
    static constexpr tas_size sc_ft5p_index = 1;

    tas_application(tas_application const &) = delete;
    tas_application & operator=(tas_application const &) = delete;
    tas_application(tas_size _max_wait_timeout);

    ~tas_application();    

    tas_error stop(tas_size _timeout) noexcept;
    bool is_running() noexcept;

    tas_overlapped_server & overlapped_server() noexcept;
    tas_error error() noexcept;
    tas_size tick_time() noexcept;
    tas_size current_time() noexcept;

    template<typename ... Modules>
    bool run(Modules & ... _modules) noexcept;
private:
    template<typename ... Modules>
    void tick(Modules & ... _modules) noexcept;
    template<typename ... Modules>
    bool start_modules(Modules & ... _modules) noexcept;
    template<typename ... Modules>
    void stop_modules(Modules & ... _modules) noexcept;
    template<typename Module>
    bool start_module(Module & _module) noexcept;


    bool init_run() noexcept;
    bool wait_next() noexcept;
    void reset_stop_event() noexcept;
    void set_stop_event() noexcept;
    void restore_stop_event() noexcept;
    void stop_event() noexcept;
    void set_owned_thread() noexcept;
    bool is_owned_thread() noexcept;
    bool ready() noexcept;
    //tas_error stop_st() noexcept;
    //tas_error stop_mt() noexcept;

    static void stop_signal_receive(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);
    //static void controll_connect_receive(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e);
    //tas_controller m_controllers[2];
    tas_overlapped_server m_osrv;
    tas_size m_max_timeout;
    tas_handle m_stopped_event;
    tas_handle m_owned_thread;
    tas_named_pipe m_stop_pipe;
    tas_error m_error;
    bool m_running;
    bool m_stop_mech_ready;
};


template<typename ... Modules>
inline bool
tas_application::run(
    Modules & ... _modules) noexcept
{
    static_assert(sizeof...(_modules) != 0, "No modules");
    //todo: log
    if (!(init_run() && start_modules(_modules...)))
    {
        return false;
    }
    //todo: log
    m_running = true;
    while (m_running)
    {
        //todo: log trace
        tick(_modules...);
        //todo: log trace
        if (!wait_next())
        {
            m_running = false;
        }        
    }
    //todo: log debug
    m_osrv.abort_and_wait_all();
    //todo: log debug
    stop_modules(_modules...);
    //todo: log debug
    set_stop_event();
    //todo: log debug
    return error() == 0;
}


template<typename ...Modules>
inline void 
tas_application::tick(Modules & ..._modules) noexcept
{
    static_assert(sizeof...(_modules) != 0, "No modules");
    return (_modules.tick(*this), ...);
}

template<typename ...Modules>
inline bool 
tas_application::start_modules(Modules & ... _modules) noexcept
{
    static_assert(sizeof...(_modules) != 0, "No modules");
    m_error = 0;
    if ((... && this->start_module(_modules)))
    {
        return true;
    }
    stop_modules(_modules...);
    return false;
}


template<typename ...Modules>
inline void 
tas_application::stop_modules(Modules & ... _modules) noexcept
{
    static_assert(sizeof...(_modules) != 0, "No modules");
    return (_modules.stop(*this), ...);
}


template<typename Module>
bool tas_application::start_module(Module & _module) noexcept
{
    m_error = _module.start(*this);
    return m_error == 0;
}


#endif //INCLUDE_TAS_APPLICATION_H
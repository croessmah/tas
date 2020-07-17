#pragma once
#ifndef INCLUDE_TAS_MODULE_IPC_H
#define INCLUDE_TAS_MODULE_IPC_H

#include <utility>
#include "types.h"
#include "pipe.h"
#include "client.h"


class tas_md_ipc
{
public:
    tas_md_ipc(tas_controller * _controllers, unsigned _count, tas_size _max_silent_time) noexcept;
    tas_error start(tas_application & _app) noexcept;
    void stop(tas_application & _app) noexcept;
    void tick(tas_application & _app) noexcept;
private:
    template<size_t ... Indexes>
    tas_md_ipc(tas_controller * _controllers, unsigned _count, tas_size _max_silent_time, std::index_sequence<Indexes...>) noexcept:
        m_clients { tas_client(_controllers, (Indexes, _count))... },
        m_max_silent_time(_max_silent_time)
    {
    }
    static constexpr unsigned sc_max_clients = tas_client::sc_pipes_options.max_instances;
    static_assert(sc_max_clients <= tas_client::sc_pipes_options.max_instances, "Too many clients");
    tas_client m_clients[sc_max_clients];
    tas_size m_max_silent_time;
};


#endif //INCLUDE_TAS_MODULE_IPC_H
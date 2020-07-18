#include "application.h"
#include "module_ipc.h"


tas_md_ipc::tas_md_ipc(tas_controller * _controllers, unsigned _count, tas_size _max_silent_time) noexcept:
    tas_md_ipc(
        _controllers, 
        _count, 
        _max_silent_time, 
        std::make_index_sequence<sizeof(m_clients) / sizeof(*m_clients)>()
    )
{    
}


tas_error tas_md_ipc::start(tas_application & _app) noexcept
{
    for (auto & client: m_clients)
    {
        if (!client.to_work(_app.overlapped_server()))
        {
            return client.last_error();
        }
    }
    return 0;
}


void tas_md_ipc::stop(tas_application &) noexcept
{
    for (auto & e : m_clients)
    {
        e.close_channel();
    }
}


void tas_md_ipc::tick(tas_application & _app) noexcept
{
    for (auto & client : m_clients)
    {
        if (client.connected() &&
            m_max_silent_time < (_app.current_time() - client.last_time()))
        {
            client.abort();
        }
    }
}

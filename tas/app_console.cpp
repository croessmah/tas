#include "app_console.h"

tas_app_console::tas_app_console(unsigned _tick_time) noexcept :
    m_controllers{
        "192.168.0.112",
        "192.168.0.111"
    },
    m_app(_tick_time),
    m_sniffer(m_app, m_controllers, 2),
    m_ipc(m_controllers, 2, 2000)
{
}

int tas_app_console::run() noexcept
{
    return m_app.run(m_sniffer, m_ipc) ? 
        EXIT_SUCCESS : 
        EXIT_FAILURE ;
}

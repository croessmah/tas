#include <WinSock2.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <cassert>
#include "module_sniffer.h"
#include "application.h"

#if !defined(WPCAP)
#define WPCAP
#endif

#if !defined(HAVE_REMOTE)
#define HAVE_REMOTE
#endif

#include <pcap.h>



namespace
{
    pcap_addr_t * find_address_in_list(pcap_addr_t * _list, uint32_t _ip)
    {
        struct sockaddr_in addr;
        for (; _list != NULL; _list = _list->next) {
            if (_list->addr->sa_family == AF_INET) {
                memcpy(&addr, _list->addr, sizeof(addr));
                if (_ip == addr.sin_addr.S_un.S_addr) {
                    break;
                }
            }
        }
        return _list;
    }
}



tas_md_sniffer::tas_md_sniffer(
    tas_application & _app, 
    tas_controller * _ctl, 
    tas_size _ctl_count) noexcept:
    m_controllers(_ctl),
    m_controllers_count(_ctl_count),
    m_adapter_ip(0),
    m_restart_time(GetTickCount()),
    m_restart_period(1000),
    m_packet_count_on_tick(_app.tick_time() / 50 + 1),
    m_buffer(nullptr),
    m_buffer_size(0),
    m_capture_device(nullptr)
{
}

tas_md_sniffer::~tas_md_sniffer() noexcept
{
    close_capture_device();
    if (m_buffer)
    {
        HeapFree(GetProcessHeap(), 0, m_buffer);
    }
}


tas_error 
tas_md_sniffer::start(tas_application & _app) noexcept
{
    return ERROR_SUCCESS;
}


void tas_md_sniffer::stop(tas_application & _app) noexcept
{
    close_capture_device();
}


void 
tas_md_sniffer::tick(tas_application & _app) noexcept
{    
    if (restart_if_needed())
    {
        capture_packets();
    }    
}


bool 
tas_md_sniffer::need_sniffer_restart() noexcept
{
    return !valid_capture_device() &&
        restart_period_elapsed() &&
        scada_running();
}


bool 
tas_md_sniffer::scada_running() noexcept
{
    return true;//todo: check scada application
}


bool 
tas_md_sniffer::restart_period_elapsed() noexcept
{
    DWORD tick_count = GetTickCount();
    if (m_restart_period < (tick_count - m_restart_time))
    {
        m_restart_time = tick_count;
        return true;
    }
    return false;
}


bool 
tas_md_sniffer::update_adapter_ip() noexcept
{    
    return find_tcp_connection();
}


bool 
tas_md_sniffer::restart_if_needed() noexcept
{
    if (need_sniffer_restart())
    {
        return create_capture_device();
    }
    return valid_capture_device();
}


bool 
tas_md_sniffer::create_capture_device() noexcept
{
    assert(m_capture_device == nullptr && "previous device not closed");
    if (!update_adapter_ip())
    {
        return false;
    }
    char error_text_buffer[PCAP_ERRBUF_SIZE];
    char src_if_string[] = PCAP_SRC_IF_STRING;
    pcap_if_t * devices_list = nullptr;
    if (-1 == pcap_findalldevs_ex(src_if_string, NULL, &devices_list, error_text_buffer)) {
        //todo: log error
        return false;//may be ommit
    }
    for (pcap_if_t * device = devices_list; device != nullptr; device = device->next) {
        pcap_addr_t * address = find_address_in_list(device->addresses, m_adapter_ip);
        if (address) {
            m_capture_device = pcap_open_live(device->name, 65535, 1, 1, error_text_buffer);
            //todo: log error
            pcap_freealldevs(devices_list);
            break;
        }
    }
    return m_capture_device != nullptr;
}


bool 
tas_md_sniffer::valid_capture_device() noexcept
{
    return m_capture_device != nullptr;
}


void 
tas_md_sniffer::capture_packets() noexcept
{
    for (unsigned cnt = 0; cnt < m_packet_count_on_tick && valid_capture_device(); ++cnt)
    {
        char const * buf;
        tas_size size;
        int res = next_packet(buf, size);
        if (res == 1)
        {
            process_packet(buf, size);
        }
        else
        {
            if (res != 0)
            {
                close_capture_device();
            }
            break;
        }             
    }
}


int 
tas_md_sniffer::next_packet(char const *& _buf, tas_size & _size) noexcept
{    
    pcap_pkthdr * header;
    const u_char * pkt_data;
    int res = pcap_next_ex(m_capture_device, &header, &pkt_data);
    if (res == 1)
    {
        _buf = reinterpret_cast<char const *>(pkt_data);
        _size = header->caplen;
    }
    return res;
}

void tas_md_sniffer::process_packet(char const * _buf, tas_size _size) noexcept
{
}


bool 
tas_md_sniffer::find_tcp_connection() noexcept
{
    alignas(alignof(MIB_TCPTABLE)) char local_buffer[sizeof(PMIB_TCPTABLE) + sizeof(MIB_TCPROW) * 32];
    void * buffer = (m_buffer == nullptr) ? local_buffer : m_buffer;
    DWORD buffer_size = (m_buffer == nullptr)? sizeof(local_buffer): m_buffer_size;
    DWORD tcp_result = 0;    
    while ((tcp_result = GetTcpTable((PMIB_TCPTABLE)buffer, &buffer_size, FALSE)) == ERROR_INSUFFICIENT_BUFFER)
    {    
        buffer = (m_buffer == nullptr) ? 
            HeapAlloc(GetProcessHeap(), 0, buffer_size) :
            HeapReAlloc(GetProcessHeap(), 0, m_buffer, buffer_size);
        if (!buffer)
        {
            break;
        }
        m_buffer = buffer;
        m_buffer_size = buffer_size;
    }

    if (tcp_result != NO_ERROR)
    {
        return false;
    } 

    PMIB_TCPTABLE tcp_table = (PMIB_TCPTABLE)buffer;

    for (DWORD i = 0; i < tcp_table->dwNumEntries; ++i)
    {
        PMIB_TCPROW row = &tcp_table->table[i];
        for (unsigned i = 0; i < m_controllers_count; ++i)
        {
            if (row->dwRemoteAddr == m_controllers[i].ip() && 
                row->State == MIB_TCP_STATE_ESTAB)
            {
                m_adapter_ip = row->dwLocalAddr;
                return true;
            }
        }
    }
    return false;
}

void tas_md_sniffer::close_capture_device() noexcept
{
    if (m_capture_device)
    {
        pcap_close(m_capture_device);
        m_capture_device = nullptr;
    }
}

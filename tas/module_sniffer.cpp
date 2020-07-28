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


    char * ip_to_str(uint32_t _ip, char * _buf, unsigned _size) noexcept
    {
        assert("buffer is too small" && _size >= 16);
        IN_ADDR addr;
        addr.S_un.S_addr = _ip;
        inet_ntop(AF_INET, &addr, _buf, _size);
        return _buf;
    }

    uint64_t make_time()
    {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        return st.wYear * 10000000000000ull +
            st.wMonth   * 100000000000ull   +
            st.wDay     * 1000000000ull     +
            st.wHour    * 10000000ull       +
            st.wMinute  * 100000ull         +
            st.wSecond  * 1000ull           +
            st.wMilliseconds;
    }
}


tas_md_sniffer::tas_md_sniffer(
    tas_application & _app, 
    tas_controller * _ctl, 
    tas_size _ctl_count) noexcept:
    m_controllers(_ctl),
    m_controllers_count(_ctl_count),
    m_adapter_ip(0),
    m_restart_timer(500),
    m_no_packet_timer(10000),
    m_packet_count_on_tick(_app.tick_time() / 50 + 1),
    m_tcp_table(nullptr),
    m_tcp_table_size(0),
    m_capture_device(nullptr)
{
}


tas_md_sniffer::~tas_md_sniffer() noexcept
{
    close_capture_device();
    if (m_tcp_table)
    {
        HeapFree(GetProcessHeap(), 0, m_tcp_table);
    }
}


tas_error 
tas_md_sniffer::start(tas_application & _app) noexcept
{
    m_last_tick_time = _app.current_time();
    m_restart_timer.reset();
    m_no_packet_timer.reset();
    return ERROR_SUCCESS;
}


void tas_md_sniffer::stop(tas_application &) noexcept
{
    close_capture_device();
}


void 
tas_md_sniffer::tick(tas_application & _app) noexcept
{
    tas_size delta = _app.current_time() - m_last_tick_time;
    if (restart_if_needed(delta))
    {
        capture_packets();
    }
    m_last_tick_time = _app.current_time();
}


bool
tas_md_sniffer::restart_if_needed(tas_size _delta) noexcept
{
    if (need_sniffer_restart(_delta))
    {
        return create_capture_device();
    }
    return valid_capture_device();
}


bool 
tas_md_sniffer::need_sniffer_restart(tas_size _delta) noexcept
{
    if (valid_capture_device())
    {
        return no_packets_timeout_elapsed(_delta);
    }
    return restart_period_elapsed(_delta) && scada_running();
}


bool
tas_md_sniffer::restart_period_elapsed(tas_size _delta) noexcept
{
    return m_restart_timer.update(_delta);
}


bool tas_md_sniffer::no_packets_timeout_elapsed(tas_size _delta) noexcept
{
    return m_no_packet_timer.update(_delta);
}


bool 
tas_md_sniffer::scada_running() noexcept
{
    return true;//todo: check scada application
}


bool 
tas_md_sniffer::update_adapter_ip() noexcept
{    
    return find_tcp_connection();
}


bool 
tas_md_sniffer::create_capture_device() noexcept
{
    close_capture_device();
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
            u_int netmask = ((struct sockaddr_in *)(device->addresses->netmask))->sin_addr.S_un.S_addr;
            char filter[64];
            generate_filter(filter, sizeof(filter));
            if (!set_capture_filter(netmask, filter))
            {
                close_capture_device();
            }
            m_no_packet_timer.reset();
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
        uint64_t time;
        int res = next_packet(buf, size, time);
        if (res == 1)
        {
            process_packet(buf, size, time);
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
tas_md_sniffer::next_packet(char const *& _buf, tas_size & _size, uint64_t & _time) noexcept
{    
    pcap_pkthdr * header;
    const u_char * pkt_data;
    int res = pcap_next_ex(m_capture_device, &header, &pkt_data);
    if (res == 1)
    {
        _buf = reinterpret_cast<char const *>(pkt_data);
        _size = header->caplen;
        _time = make_time();
    }
    return res;
}


void tas_md_sniffer::process_packet(char const * _buf, tas_size _size, uint64_t _time) noexcept
{
    for (unsigned i = 0; i < m_controllers_count; ++i)
    {
        if (m_controllers[i].update(_buf, static_cast<unsigned>(_size), _time))
        {
            m_no_packet_timer.reset();
            break;
        }
    }
}


bool 
tas_md_sniffer::find_tcp_connection() noexcept
{
    alignas(alignof(MIB_TCPTABLE)) char local_buffer[64 * sizeof(MIB_TCPROW) + sizeof(MIB_TCPTABLE)];
    void * buffer = (m_tcp_table == nullptr) ? local_buffer : m_tcp_table;
    DWORD buffer_size = (m_tcp_table == nullptr)? sizeof(local_buffer): m_tcp_table_size;
    DWORD tcp_result = 0;
    while ((tcp_result = GetTcpTable((PMIB_TCPTABLE)buffer, &buffer_size, FALSE)) == ERROR_INSUFFICIENT_BUFFER)
    {
        buffer = (m_tcp_table == nullptr) ? 
            HeapAlloc(GetProcessHeap(), 0, buffer_size) :
            HeapReAlloc(GetProcessHeap(), 0, m_tcp_table, buffer_size);
        if (!buffer)
        {
            break;
        }
        m_tcp_table = buffer;
        m_tcp_table_size = buffer_size;
    }

    if (tcp_result != NO_ERROR)
    {
        return false;
    } 

    PMIB_TCPTABLE tcp_table = (PMIB_TCPTABLE)buffer;

    for (DWORD i = 0; i < tcp_table->dwNumEntries; ++i)
    {
        PMIB_TCPROW row = &tcp_table->table[i];
        for (unsigned j = 0; j < m_controllers_count; ++j)
        {
            if (row->dwRemoteAddr == m_controllers[j].ip() && 
                row->State == MIB_TCP_STATE_ESTAB)
            {
                m_adapter_ip = row->dwLocalAddr;
                return true;
            }
        }
    }
    return false;
}


bool tas_md_sniffer::set_capture_filter(uint32_t _netmask, char const * _filter) noexcept
{
    assert(m_capture_device != nullptr && "invalid device");
    bool result = false;
    bpf_program fcode;
    if (pcap_compile(m_capture_device, &fcode, _filter, true, _netmask) != -1)
    {
        result = (pcap_setfilter(m_capture_device, &fcode) == 0);
        pcap_freecode(&fcode);
    }    
    return result;
}


void tas_md_sniffer::generate_filter(char * _buf, tas_size _buf_size) noexcept
{
    //"src 111.111.111.111 or src 111.111.111.111"
    assert("buffer is too small" && (_buf_size >= m_controllers_count * 20 + 4 * (m_controllers_count - 1)));
    (void)_buf_size;
    *_buf = '\0';
    for (unsigned i = 0; i < m_controllers_count; ++i)
    {
        char buffer[24];
        char const * prefix = (i != 0) ? " or src " : "src ";
        unsigned const length = strlen(prefix);
        strcpy(buffer, prefix);
        ip_to_str(m_controllers[i].ip(), buffer + length, sizeof(buffer) - length);
        strcat(_buf, buffer);
    }
}


void tas_md_sniffer::close_capture_device() noexcept
{
    if (m_capture_device)
    {
        pcap_close(m_capture_device);
        m_capture_device = nullptr;
    }
}

#pragma once
#ifndef INCLUDE_TAS_MODULE_SNIFFER_H
#define INCLUDE_TAS_MODULE_SNIFFER_H

#include <cinttypes>
#include "types.h"
#include "controller.h"

class tas_application;

class tas_md_sniffer
{
    tas_md_sniffer(tas_md_sniffer const &) = delete;
    tas_md_sniffer & operator=(tas_md_sniffer const &) = delete;
public:
    tas_md_sniffer(tas_application & _app, tas_controller * _ctl, tas_size _ctl_count) noexcept;
    ~tas_md_sniffer() noexcept;
    tas_error start(tas_application & _app) noexcept;
    void stop(tas_application & _app) noexcept;
    void tick(tas_application & _app) noexcept;
private:
    bool need_sniffer_restart() noexcept;
    bool scada_running() noexcept;
    bool restart_period_elapsed() noexcept;
    bool update_adapter_ip() noexcept;
    bool restart_if_needed() noexcept;
    bool create_capture_device() noexcept;
    bool valid_capture_device() noexcept;
    void capture_packets() noexcept;
    int next_packet(char const *& _buf, tas_size & _size) noexcept;
    void process_packet(char const * _buf, tas_size _size) noexcept;
    bool find_tcp_connection() noexcept;
    void close_capture_device() noexcept;
    tas_controller * m_controllers;
    tas_size m_controllers_count;
    uint32_t m_adapter_ip;
    tas_size m_restart_time;
    tas_size m_restart_period;
    unsigned m_packet_count_on_tick;
    void * m_buffer;
    tas_size m_buffer_size;
    struct pcap * m_capture_device;
};

#endif //INCLUDE_TAS_MODULE_SNIFFER_H
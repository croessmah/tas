#pragma once
#ifndef INCLUDE_TAS_APP_CONSOLE_H
#define INCLUDE_TAS_APP_CONSOLE_H

#include "application.h"
#include "controller.h"
#include "module_sniffer.h"
#include "module_ipc.h"


class tas_app_console
{
    tas_app_console(tas_app_console const &) = delete;
    tas_app_console & operator=(tas_app_console const &) = delete;
public:
    tas_app_console(unsigned _tick_time);
    int run();
private:
    tas_controller m_controllers[2];
    tas_application m_app;
    tas_md_sniffer m_sniffer;
    tas_md_ipc m_ipc;
};



#endif //INCLUDE_TAS_APP_CONSOLE_H
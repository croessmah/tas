#pragma once
#ifndef INCLUDE_TAS_APP_SERVICE_H
#define INCLUDE_TAS_APP_SERVICE_H


class tas_app_service
{
    tas_app_service(tas_app_service const &) = delete;
    tas_app_service & operator=(tas_app_service const &) = delete;
public:
    tas_app_service(unsigned _tick_time) noexcept;
    int run() noexcept;

    wchar_t const * service_name() const noexcept;
private:
    wchar_t m_service_name[128];
};


#endif //INCLUDE_TAS_APP_SERVICE_H

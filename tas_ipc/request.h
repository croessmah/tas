#pragma once
#ifndef INCLUSE_TAS_IPC_REQUEST_H
#define INCLUSE_TAS_IPC_REQUEST_H
#include <string>
#include <climits>
#include "exceptions.h"
#include "tas_ipc.h"

namespace tas
{



class request
{
public:
    static constexpr unsigned sc_max_request_size = TAS_MAX_REQUEST_SIZE;
    static constexpr unsigned sc_infinite = UINT_MAX;
    request();
    std::string const & send(unsigned _timeout, bool _repeat_if_busy);
    void clear() noexcept;
    bool add_number(uint16_t _num);
    std::string const & get_request() const noexcept
    {
        return m_request_text;
    }
    std::string const & get_last_ansfer() const noexcept
    {
        return m_last_ansfer_text;
    }
private:
    std::string m_request_text;
    std::string m_last_ansfer_text;
};


TAS_INTRODUCE_GENERAL_EXCEPTION(request_overflow, TAS_ERR_REQUEST_OVERFLOW);
TAS_INTRODUCE_GENERAL_EXCEPTION(request_timedout, TAS_ERR_REQUEST_TIMEDOUT);
TAS_INTRODUCE_GENERAL_EXCEPTION(request_invalid, TAS_ERR_REQUEST_INVALID);
TAS_INTRODUCE_GENERAL_EXCEPTION(request_no_connection, TAS_ERR_CONNECTION_ERROR);


}//namespace tas



#endif //INCLUSE_TAS_IPC_REQUEST_H
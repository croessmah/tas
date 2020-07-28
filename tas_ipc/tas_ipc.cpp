#include <memory>
#include "query.h"
#include "tas_ipc.h"


namespace
{
    template<typename Function>
    inline unsigned safe_call(Function _f) noexcept
    {
        unsigned error = TAS_ERR_SUCCESS;
        try
        {
            error = _f();
        }
        catch (tas::exception const & except)
        {
            error = except.code();
        }
        catch (std::bad_alloc const &)
        {
            error = TAS_ERR_BAD_ALLOCATION;
        }
        catch (...)
        {
            error = TAS_ERR_UNEXPECTED;
        }
        return error;
    }
}//internal linkage



tas_query_handle tas_query_create_by_name(char const * _ctl_name, unsigned * _e) TAS_IPC_NOEXCEPT
{
    unsigned error = TAS_ERR_SUCCESS;
    tas::query * result = nullptr;
    auto func = [&result, _ctl_name]()
    {
        if (!_ctl_name)
        {
            return TAS_ERR_INVALID_PARAMETER;
        }
        result = new tas::query(_ctl_name);
        return TAS_ERR_SUCCESS;        
    };
    error = safe_call(func);
    if (_e)
    {
        *_e = error;
    }
    return result;
}


void tas_query_destroy(tas_query_handle _query) TAS_IPC_NOEXCEPT
{
    delete _query;
}


TAS_IPC_API unsigned tas_query_add_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx) TAS_IPC_NOEXCEPT
{
    if (!_query)
    {
        return TAS_ERR_INVALID_PARAMETER;
    }
    auto func = [query = static_cast<tas::query *>(_query), _pdx, _vdx]()
    {
        return query->add_param(_pdx, _vdx) ? 
            TAS_ERR_SUCCESS : 
            TAS_ERR_ALREADY_CONTAINED ;
    };
    return safe_call(func);
}


TAS_IPC_API unsigned tas_query_apply(tas_query_handle _query, unsigned _timeout, int64_t * _update_lost) TAS_IPC_NOEXCEPT
{
    if (!_query || !_update_lost)
    {
        return TAS_ERR_INVALID_PARAMETER;
    }
    auto func = [=]()
    {
        auto query = static_cast<tas::query *>(_query);
        *_update_lost = query->request(_timeout);
        return TAS_ERR_SUCCESS;
    };
    return safe_call(func);
}


TAS_IPC_API unsigned tas_query_get_param(tas_query_handle _query, uint16_t _pdx, uint16_t _vdx, char * _result, unsigned * _written) TAS_IPC_NOEXCEPT
{
    if (!_query || !_result || !_written)
    {
        return TAS_ERR_INVALID_PARAMETER;
    }
    auto func = [=]()
    {
        std::string_view value = static_cast<tas::query *>(_query)->get_param(_pdx, _vdx);
        *_written = static_cast<unsigned>(value.size());
        _result[*_written] = '\0';
        if (*_written)
        {        
            value.copy(_result, *_written);
            return TAS_ERR_SUCCESS;
        }
        return TAS_ERR_VALUE_NOT_FOUND;
    };
    return safe_call(func);
}


TAS_IPC_API void tas_query_clear(tas_query_handle _query) TAS_IPC_NOEXCEPT
{
    if (!_query)
    {
        static_cast<tas::query *>(_query)->clear();
    }
}
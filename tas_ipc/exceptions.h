#pragma once
#ifndef INCLUDE_TAS_EXCEPTIONS_H
#define INCLUDE_TAS_EXCEPTIONS_H
#include <stdexcept>

namespace tas
{
    class exception : std::runtime_error
    {
    public:
        exception(unsigned _code, char const * _text):
            std::runtime_error(_text),
            m_code(_code)
        {}
        unsigned code() const noexcept { return m_code; }
    private:
        unsigned m_code;
    };


    template<unsigned Code>
    struct e_general_exception: exception
    {
        e_general_exception(char const * _text): exception(Code, _text) {}
    };

#define TAS_INTRODUCE_GENERAL_EXCEPTION(name, code)         \
    struct name: ::tas::e_general_exception<code>           \
    {                                                       \
        using base_t = ::tas::e_general_exception<code>;    \
        using base_t::base_t;                               \
        name(): base_t(#name) {}                            \
    }

}//namespace tas


#endif //INCLUDE_TAS_EXCEPTIONS_H
#pragma once
#include <utility>
#include "types.h"

class tas_overlapped
{
    tas_overlapped(tas_overlapped const &) = delete;
    tas_overlapped & operator=(tas_overlapped const &) = delete;
public:
    tas_overlapped() noexcept;
    tas_overlapped(bool _state) noexcept;
    ~tas_overlapped() noexcept;
    tas_handle event() const noexcept;
    bool is_valid_event() const noexcept;
    tas_error get_result(tas_handle _file, tas_size & _written, bool _wait = false) noexcept;
    struct _OVERLAPPED * raw() noexcept;
    struct _OVERLAPPED const * raw() const noexcept;
private:
    constexpr static unsigned gc_overlapped_size = sizeof(void *) * 4 + (8 - sizeof(void *));
    std::aligned_storage_t<gc_overlapped_size, alignof(void *)> m_storage;
};


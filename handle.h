#ifndef HANDLE__H_
#define HANDLE__H_
#include <Windows.h>

namespace clre {

struct non_zero {
    template<typename T>
    static bool call(const T &handle) noexcept {
        return (intptr_t) handle != 0;
    }
};

struct non_negative {
    template<typename T>
    static bool call(const T &handle) noexcept {
        return (intptr_t) handle > 0;
    }
};

template<typename wrapped_t>
struct with_pseudo {
    template<typename T>
    static bool call(const T &handle) noexcept {
        if (wrapped_t::call(handle))
            return true;

        // Check if it's a pseudo handle
        auto h = (HANDLE)(intptr_t)handle;
        return h == GetCurrentProcess() || h == GetCurrentThread();
    }
};

// Strong exception guarantee
template<typename handle_t, typename Fn, Fn close, typename check_valid = non_negative>
class HandleGuard
{
private:
    handle_t _handle;
public:
    static constexpr handle_t zero_handle{ };

public:
    explicit HandleGuard( handle_t handle = zero_handle ) noexcept
        : _handle(handle) { }

    HandleGuard& operator =(handle_t handle) noexcept {
        reset(handle);
        return *this;
    }

    ~HandleGuard() {
        if (non_negative::call(_handle))
            close(_handle);
    }

    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator =(const HandleGuard&) = delete;
    
    HandleGuard( HandleGuard &&rhs ) noexcept
        : _handle(rhs._handle) {
        rhs._handle = zero_handle;
    }
    HandleGuard& operator =(HandleGuard &&rhs) noexcept {
        if (&rhs == this)
            return *this;

        reset(rhs._handle);
        rhs._handle = zero_handle;

        return *this;
    }

    void reset(handle_t handle = zero_handle) noexcept {
        if (handle == _handle)
            return;
        if (non_negative::call(_handle))
            close(_handle);
        _handle = handle;
    }

    handle_t release() noexcept {
        auto tmp = _handle;
        _handle = zero_handle;
        return tmp;
    }

    handle_t get() const noexcept { return _handle; }
    bool valid() const noexcept { return check_valid::call(_handle); }

    operator handle_t() const noexcept { return _handle; }
    explicit operator bool() const noexcept { return valid(); }
    handle_t *operator &() noexcept { return &_handle; }
    const handle_t *operator &() const noexcept { return &_handle; }

    bool operator ==(const HandleGuard& rhs) const noexcept { return _handle == rhs._handle; }
    bool operator <(const HandleGuard& rhs) const noexcept { return _handle < rhs._handle; }
};

template<typename handle_t, typename Fn, Fn close, typename check_valid>
constexpr handle_t HandleGuard<handle_t, Fn, close, check_valid>::zero_handle; 

using Handle        = HandleGuard<HANDLE, decltype(&CloseHandle), &CloseHandle>;
using ProcessHandle = HandleGuard<HANDLE, decltype(&CloseHandle), &CloseHandle, with_pseudo<non_negative> >;
using ACtxHandle    = HandleGuard<HANDLE, decltype(&ReleaseActCtx), &ReleaseActCtx>;
using RegHandle     = HandleGuard<HKEY, decltype(&RegCloseKey), &RegCloseKey>;
using Mapping       = HandleGuard<void*, decltype(&UnmapViewOfFile), &UnmapViewOfFile, non_zero>;

}
#endif

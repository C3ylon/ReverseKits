#ifndef FUNDAMENTAL__H_
#define FUNDAMENTAL__H_
#include "header.h"
namespace clre {

inline size_t ReadMemory(HANDLE hProcess, const void *dst_addr, void *buffer, size_t size); 
inline size_t WriteMemory(HANDLE hProcess, void *dst_addr, const void *buffer, size_t size); 


size_t ReadMemory(HANDLE hProcess, const void *dst_addr, void *buffer, size_t size) {
    DWORD oldprotect;
    size_t BytesRead;
    VirtualProtectEx(hProcess, (void*)dst_addr, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    ReadProcessMemory(hProcess, dst_addr, buffer, size, &BytesRead);
    VirtualProtectEx(hProcess, (void*)dst_addr, size, oldprotect, &oldprotect);
    return BytesRead;
}

size_t WriteMemory(HANDLE hProcess, void *dst_addr, const void *buffer, size_t size) {
    DWORD oldprotect;
    size_t BytesWritten;
    VirtualProtectEx(hProcess, dst_addr, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    WriteProcessMemory(hProcess, dst_addr, buffer, size, &BytesWritten);
    VirtualProtectEx(hProcess, dst_addr, size, oldprotect, &oldprotect);
    return BytesWritten;
}

}


#endif

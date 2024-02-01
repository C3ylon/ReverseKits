#ifndef FUNDAMENTAL__H_
#define FUNDAMENTAL__H_
#include "header.h"
namespace clre {
    
inline size_t WriteMemory(HANDLE hProcess, void *dst_addr, const void *buffer, size_t size); 

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
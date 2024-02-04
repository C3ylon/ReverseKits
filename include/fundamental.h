#ifndef FUNDAMENTAL__H_
#define FUNDAMENTAL__H_
#include "header.h"
namespace clre {

inline size_t ReadMemory(HANDLE hProcess, const void *dst_addr, void *buffer, size_t size); 
inline size_t WriteMemory(HANDLE hProcess, void *dst_addr, const void *buffer, size_t size); 

inline void InjectDll(HANDLE hProcess, const char *dllpath);
inline void EjectDll(HANDLE hProcess, DWORD pid, const char *dllname);
inline void *GetBaseAddress(HANDLE hProcess);

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

void InjectDll(HANDLE hProcess, const char *dllpath) {
    const auto pathsize = string(dllpath).length() + 1; 
    auto pRemotebuffer = VirtualAllocEx(hProcess, nullptr, pathsize, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, pRemotebuffer, dllpath, pathsize, nullptr);
    auto hMod = GetModuleHandleA("kernel32.dll");
    auto thread_proc = GetProcAddress(hMod, "LoadLibraryA");
    if(thread_proc == nullptr) {
        VirtualFreeEx(hProcess, pRemotebuffer, 0, MEM_RELEASE);
        throw std::runtime_error("GetProcAddress LoadLibraryA failed");
    }
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, *(LPTHREAD_START_ROUTINE *)&thread_proc, pRemotebuffer, 0, nullptr);
    if(hThread == nullptr) {
        VirtualFreeEx(hProcess, pRemotebuffer, 0, MEM_RELEASE);
        throw std::runtime_error("CreateRemoteThread failed");
    }
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, pRemotebuffer, 0, MEM_RELEASE);
    CloseHandle(hThread);
}

void EjectDll(HANDLE hProcess, DWORD pid, const char *dllname) {
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    bool found = false;
    bool more = Module32First(hSnapshot, &me);
    for( ; more; more = Module32Next(hSnapshot, &me)) {
        if(string(me.szModule) == string(dllname)
            || string(me.szExePath) == string(dllname)) {
                found = true;
                break;
            }
    }
    CloseHandle(hSnapshot);
    if(found == false)
        throw std::runtime_error("Find Module Entry failed");
    auto hMod = GetModuleHandleA("kernel32.dll");
    auto thread_proc = GetProcAddress(hMod, "FreeLibrary");
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, *(LPTHREAD_START_ROUTINE *)&thread_proc, me.modBaseAddr, 0, nullptr);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

void *GetBaseAddress(HANDLE hProcess) {
    HMODULE lphModule[1024] = { };
    DWORD lpcbNeeded = 0;
    if (EnumProcessModulesEx(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded, LIST_MODULES_ALL) == false) {
        throw std::runtime_error("EnumProcessModules failed");
    }
    if(lpcbNeeded == 0)
        throw std::runtime_error("EnumProcessModules get none module handle");
    return (void*)lphModule[0];
}


}


#endif

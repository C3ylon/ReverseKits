#ifndef FUNDAMENTAL__H_
#define FUNDAMENTAL__H_
#include "header.h"
namespace clre {

inline size_t ReadMemory(HANDLE hProcess, const void *dst_addr, void *buffer, size_t size); 
inline size_t WriteMemory(HANDLE hProcess, void *dst_addr, const void *buffer, size_t size); 

inline MODULEENTRY32 GetModInfo(DWORD pid, const char *dllname);
inline void InjectDll(HANDLE hProcess, const char *dllpath);
inline void EjectDll(HANDLE hProcess, DWORD pid, const char *dllname);
inline void *GetBaseAddress(HANDLE hProcess);
inline void *GetProcAddressEx64(HANDLE hProcess, HMODULE hModule, const char *lpProcName);

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

MODULEENTRY32 GetModInfo(DWORD pid, const char *dllname) {
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
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
    return me;
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
    auto me = GetModInfo(pid, dllname);
    auto hMod = GetModuleHandleA("kernel32.dll");
    auto thread_proc = GetProcAddress(hMod, "FreeLibrary");
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, *(LPTHREAD_START_ROUTINE *)&thread_proc, me.hModule, 0, nullptr);
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

void *GetProcAddressEx64(HANDLE hProcess, HMODULE hModule, const char *lpProcName) {
    auto real_rva = [hModule](DWORD rva) { return (void*)((size_t)hModule + rva); };
    DWORD ntoffset;
    ReadMemory(hProcess, real_rva(0x3C), &ntoffset, sizeof(DWORD));
    DWORD eat_rva;
    ReadMemory(hProcess, real_rva(ntoffset + 0x88), &eat_rva, sizeof(DWORD));
    IMAGE_EXPORT_DIRECTORY ied;
    ReadMemory(hProcess, real_rva(eat_rva), &ied, sizeof(ied));
    auto name_array_rva = (DWORD*)real_rva(ied.AddressOfNames);
    bool found = false;
    DWORD ordinal = 0;
    for( ; ordinal < ied.NumberOfNames; ordinal++) {
        DWORD name_rva;
        ReadMemory(hProcess, &name_array_rva[ordinal], &name_rva, sizeof(DWORD));
        char name[MAX_PATH];
        ReadMemory(hProcess, real_rva(name_rva), name, MAX_PATH);
        name[MAX_PATH - 1] = '\0';
        if(string(name) == string(lpProcName)) {
            found = true;
            break;
        }
    }
    if(found == false)
        throw std::runtime_error("Can't find ProcName in EAT");
    auto ordinal_arr_rva = (WORD*)real_rva(ied.AddressOfNameOrdinals);
    ReadMemory(hProcess, &ordinal_arr_rva[ordinal], &ordinal, sizeof(WORD));
    ordinal = (WORD)ordinal;
    auto funcaddr_arr_rva = (DWORD*)real_rva(ied.AddressOfFunctions);
    DWORD funcaddr_rva;
    ReadMemory(hProcess, &funcaddr_arr_rva[ordinal], &funcaddr_rva, sizeof(DWORD));
    return real_rva(funcaddr_rva);
}

}


#endif

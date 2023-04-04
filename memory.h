#ifndef MEMORY_HACK__H_
#define MEMORY_HACK__H_

#include <stdlib.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <string.h>

#include "log.h"

#ifdef LOG__H_
    #define LOG_INFO(...) log_info(__VA_ARGS__)
    #define LOG_ERROR(...) log_error(__VA_ARGS__)
#else
    #define LOG_INFO(...)
    #define LOG_ERROR(...)
#endif

#define PAGE_SIZE 4096u
#define x64

extern DWORD pid;
extern HANDLE hProcess;

int InitByPid(DWORD _pid);
int InitByWindowNameW(WCHAR *WindowName);
int InitByProcessNameW(WCHAR *ProcessName);
HMODULE GetModuleBaseAddr(WCHAR *moduleName);
int ReadMemory(SIZE_T addr, SIZE_T size, void* readbuff);
int WriteMemory(SIZE_T addr, SIZE_T size, void* writebuff);
UINT64 ScanforSignature(char *signature, UINT64 begin, UINT64 end, int ordinal);

#endif
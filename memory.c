#include "memory.h"

DWORD pid;
HANDLE hProcess;

static BOOL ISINITIALIZED;
static DWORD ThreadID;

BOOL InitByPid(DWORD _pid)
{
    pid = _pid;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    if(hProcess) {
        ISINITIALIZED = 1;
    }
    return ISINITIALIZED;
}

int InitByWindowNameW(WCHAR *WindowName)
{
    HWND hWnd = FindWindowW(NULL, (LPCWSTR)WindowName);
    ThreadID = GetWindowThreadProcessId(hWnd, (LPDWORD)&pid);
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    if(hProcess) {
        ISINITIALIZED = 1;
    }
    return ISINITIALIZED;
}


int InitByProcessNameW(WCHAR *ProcessName)
{
    PROCESSENTRY32W ProcessInfo = { 0 };
    ProcessInfo.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(!Process32FirstW(hSnapshot, &ProcessInfo)) {
        printf("[!]Process32FirstW fail\n");
        CloseHandle(hSnapshot);
        return FALSE;
    }
    do {
        if (wcscmp(ProcessName, ProcessInfo.szExeFile) == 0) {
            pid = ProcessInfo.th32ProcessID;
            break;
        }
    } while(Process32NextW(hSnapshot, &ProcessInfo));
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    if(hProcess) {
        ISINITIALIZED = 1;
    }
    return ISINITIALIZED;
}


HMODULE GetModuleBaseAddr(WCHAR *moduleName)
{
    MODULEENTRY32W moduleEntry = { 0 };
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    HMODULE res = NULL;
    if (!Module32FirstW(hSnapshot, &moduleEntry)) 
    {
        printf("[!]Module32FirstW fail\n");
        return res;
    }
    do {
        if (wcscmp(moduleEntry.szModule, moduleName) == 0) 
        {
            res = moduleEntry.hModule;
            break;
        }
    } while (Module32NextW(hSnapshot, &moduleEntry));
    CloseHandle(hSnapshot);
    return res;
}

int ReadMemory(SIZE_T addr, SIZE_T size, void* readbuff)
{
	DWORD oldprotect = 0;
	VirtualProtectEx(hProcess, (void*)addr, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	int result = ReadProcessMemory(hProcess, (void*)addr, readbuff, size, NULL);
	VirtualProtectEx(hProcess, (void*)addr, size, oldprotect, &oldprotect);
	return result;
}

int WriteMemory(SIZE_T addr, SIZE_T size, void* writebuff)
{
	DWORD oldprotect = 0;
	VirtualProtectEx(hProcess, (void*)addr, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	int result = WriteProcessMemory(hProcess, (void*)addr, writebuff, size, NULL);
	VirtualProtectEx(hProcess, (void*)addr, size, oldprotect, &oldprotect);
	return result;
}

UINT64 ScanforSignature(char *signature, UINT64 begin, UINT64 end, int ordinal)
{
    UINT64 i = 0, len = 0;
    while(signature[i++]) ++len;
    if(!len)
    {
        printf("[!]signature form wrong\n");
        return FALSE;
    }
    USHORT *byteval = (USHORT*)malloc((len / 2) * sizeof(USHORT));
    i = 0;
    while(i < len / 2) byteval[i++] = 0;
    i = 0;
    int wildcardpos = -1;
    while(i < len)
    {
        if(signature[i] == '\0')
        {
            if(i & 1)
            {
                printf("[!]signature form wrong\n");
                free(byteval);
                return FALSE;
            }
            break;
        }
        else if(signature[i] >= 'a' && signature[i] <= 'f')
            byteval[i / 2] += (signature[i] - 'a' + 10) * ((i & 1) ? 1 : 0x10), ++i;
        else if(signature[i] >= 'A' && signature[i] <= 'F')
            byteval[i / 2] += (signature[i] - 'A' + 10) * ((i & 1) ? 1 : 0x10), ++i;
        else if(signature[i] >= '0' && signature[i] <= '9')
            byteval[i / 2] += (signature[i] - '0') * ((i & 1) ? 1 : 0x10), ++i;
        else if(signature[i] == '?' || signature[i] == '.')
        {
            if(i & 1 || (signature[i + 1] != '?' && signature[i] != '.'))
            {
                printf("[!]need two consecutive wildcards\n");
                free(byteval);
                return FALSE;
            }
            byteval[i / 2] = 0xff + 1;
            wildcardpos = i / 2;
            i += 2;
        }
        else ++signature;
    }
    len = i / 2;
    USHORT *offsettabel = (USHORT*)malloc((0xff + 2) * sizeof(USHORT));
    i = 0;
    while(i < 0xff + 2) offsettabel[i++] = len - wildcardpos;
    i = wildcardpos;
    while(i < len) {
        offsettabel[byteval[i]] = len - i;
        i++;
    }
    if(end == 0) 
    {
#ifdef x86
        end = 0x7fffffff;
#endif
#ifdef x64
        end = 0x7fffffffffffffff;
#endif
    }
    if(ordinal == 0) ordinal = 1;

    UCHAR *buff = (UCHAR*)malloc(PAGE_SIZE + len);
    UINT64 readlen = 0;
    // DWORD oldprotect;
    while(begin + len <= end + 1)
    {
        readlen = begin + PAGE_SIZE + len <= end + 1 ? PAGE_SIZE + len : end + 1 - begin;
        // VirtualProtectEx(hProcess, (LPVOID)begin, readlen, PAGE_EXECUTE_READWRITE, &oldprotect);
        if(!ReadProcessMemory(hProcess, (LPVOID)begin, buff, readlen, NULL))
        {
            // printf("[!]ReadProcessMemory fail code: %x\n", GetLastError());
            return FALSE;
        }
        unsigned int i = 0, j = 0;
        while(i <= readlen - len)
        {
            if(j == len) 
            {
                if(--ordinal) 
                {
                    if(i + len == readlen) break;
                    i += offsettabel[buff[i + len]], j = 0;
                    continue;
                }
                free(buff);
                free(offsettabel);
                free(byteval);
                // VirtualProtectEx(hProcess, (LPVOID)begin, readlen, oldprotect, &oldprotect);
                return begin + i;
            }
            if(buff[i + j] == byteval[j] || byteval[j] == 256) ++j;
            else
            {
                if(i + len == readlen) break;	
                i += offsettabel[buff[i + len]], j = 0;
            }
        }
        // VirtualProtectEx(hProcess, (LPVOID)begin, readlen, oldprotect, &oldprotect);
        begin += readlen == PAGE_SIZE + len ? PAGE_SIZE : end + 1 - begin;
    }
    free(buff);
    free(offsettabel);
    free(byteval);
    return FALSE;
}

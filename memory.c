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

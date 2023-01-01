#include "memory.h"

DWORD pid;
HANDLE hProcess;

static BOOL ISINITIALIZED;

BOOL Initialize(DWORD _pid)
{
    pid = _pid;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    if(hProcess) {
        ISINITIALIZED = 1;
    }
    return ISINITIALIZED;
}
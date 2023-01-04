#ifndef MEMORY_HACK__H_
#define MEMORY_HACK__H_

#include <stdlib.h>
#include <Windows.h>

extern DWORD pid;
extern HANDLE hProcess;

int InitByPid(DWORD _pid);
int InitByWindowNameW(WCHAR *WindowName);


#endif
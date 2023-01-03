#ifndef MEMORYHACK_H
#define MEMORYHACK_H

#include <stdlib.h>
#include <Windows.h>

extern DWORD pid;
extern HANDLE hProcess;

int InitByPid(DWORD _pid);
int InitByWindowNameW(WCHAR *WindowName);


#endif
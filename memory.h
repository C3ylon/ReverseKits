#ifndef __MEMORYHACK_H
#define __MEMORYHACK_H

#include <stdlib.h>
#include <Windows.h>

extern DWORD pid;
extern HANDLE hProcess;

int InitByPid(DWORD pid);


#endif
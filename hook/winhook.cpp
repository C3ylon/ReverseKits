#include "stdio.h"
#include "conio.h"
#include "windows.h"

#define	DEF_DLL_NAME		"keyhook.dll"
#define	DEF_HOOKSTART		"HookStart"
#define	DEF_HOOKSTOP		"HookStop"

typedef HHOOK (*PFN_HOOKSTART)();
typedef void (*PFN_HOOKSTOP)();

int main() {
	HMODULE			hDll = NULL;
	PFN_HOOKSTART	HookStart = NULL;
	PFN_HOOKSTOP	HookStop = NULL;

	hDll = LoadLibraryA(DEF_DLL_NAME);
    if( hDll == NULL ) {
        printf("LoadLibrary(%s) failed!!! [%d]", DEF_DLL_NAME, (int)GetLastError());
        return 0;
    }
	auto tmp = GetProcAddress(hDll, DEF_HOOKSTART);
	HookStart = *(PFN_HOOKSTART *)&tmp;
	HookStop = (PFN_HOOKSTOP)GetProcAddress(hDll, DEF_HOOKSTOP);

	

    auto hk = HookStart();


    printf("%p\n", (void*)hk);
	printf("press 'q' to quit!\n");
	while( _getch() != 'q' )	;

	HookStop();

	FreeLibrary(hDll);
    return 0;
}
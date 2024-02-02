#include "windows.h"
#include <iostream>
#include <string>



#ifdef __cplusplus
extern "C" {
#endif
	int __stdcall MyMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
#ifdef __cplusplus
}
#endif

int __stdcall MyMessageBox(HWND hWnd, char *lpText, LPCSTR lpCaption, UINT uType) {
    DWORD oldprotect;
    VirtualProtectEx(GetCurrentProcess(), lpText, 3, PAGE_EXECUTE_READWRITE, &oldprotect);
    for(int i = 0; i < 3; i++) {
        if(lpText[i] >= 'a' && lpText[i] <= 'z') {
            lpText[i] -= 0x20;
        }
    }
    VirtualProtectEx(GetCurrentProcess(), lpText, 3, oldprotect, &oldprotect);
    return MessageBoxA(hWnd, lpText, lpCaption, uType);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved) {
    (void)hinstDLL;
    (void)lpvReserved;
    switch(dwReason) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}




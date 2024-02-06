#include "header.h"


#ifdef __cplusplus
extern "C" {
#endif
int __stdcall MyMessageBox(HWND hWnd, char *lpText, LPCSTR lpCaption, UINT uType) {
    string s = lpText;
    for(size_t i = 0; i < s.length(); i++) {
        if(s[i] >= 'a' && s[i] <= 'z') {
            s[i] -= 0x20;
        }
    }

    return MessageBoxA(hWnd, s.c_str(), lpCaption, uType);
}


#ifdef __cplusplus
}
#endif



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




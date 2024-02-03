#include "windows.h"
#include <iostream>
#include <string>

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
FILE *g_fp;


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    (void)lpvReserved;
	switch( dwReason )
	{
        case DLL_PROCESS_ATTACH: {
			g_hInstance = hinstDLL;
			break;
		}


        case DLL_PROCESS_DETACH:
			if(g_fp) fclose(g_fp);
			break;
	}

	return TRUE;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if( code == 0 )
	{
		// bit 31 : 0 => press, 1 => release
		// if( !(lParam & 0x80000000) && wParam != 0x51)
		// {
		// 	static std::string modulename;
		// 	static std::string tid;
		// 	if(g_fp == nullptr) {
		// 		char szPath[MAX_PATH] = { };
		// 		GetModuleFileNameA(NULL, szPath, MAX_PATH);
		// 		char *p = NULL;
		// 		p = strrchr(szPath, '\\');
		// 		modulename = p + 1;
		// 		tid = std::to_string(GetCurrentThreadId());
		// 		std::string filename = std::string("C:\\Users\\Ceylon\\Desktop\\ReverseKits\\hook\\") + modulename + tid + ".txt";
		// 		g_fp = fopen(filename.c_str(), "a");
		// 	}
		// 	std::string content;
		// 	content = std::string("wParam: ") + std::to_string(wParam) + "  module: " + modulename + "  thread: " + tid + "  "
		// 				+ "ncode: " + std::to_string(code) + "\n";
		// 	fputs(content.c_str(), g_fp);

        //         // return 1;
		// }
		if(wParam == 0x41) {
			if(!(lParam & (1 << 31))) {
				keybd_event(0x44, 0, 0, 0);
            	keybd_event(0x44, 0, KEYEVENTF_KEYUP, 0);
			}
			return 1;
		}
		// if(wParam == 0x44) {
		// 	if(!(lParam & (1 << 31))) {
		// 		keybd_event(0x41, 0, 0, 0);
		// 		keybd_event(0x41, 0, KEYEVENTF_KEYUP, 0);
		// 	}
		// }
	}

	return CallNextHookEx(g_hHook, code, wParam, lParam);
}

// LRESULT CALLBACK KeyboardProcLL(int code, WPARAM wParam, LPARAM lParam) {
// 	if( code == HC_ACTION ) {
		
// 		auto vk = (KBDLLHOOKSTRUCT *)lParam;
// 			std::string num = std::to_string(wParam);
// 			MessageBoxA(0, num.c_str(), 0, 0);
// 		if(wParam == WM_KEYDOWN) {

// 			// if(vk->vkCode == 0x41)
// 			// 	vk->vkCode = 0x44;
// 			// if(vk->vkCode == 0x44)
// 			// 	vk->vkCode = 0x41;
// 			// static std::string modulename;
// 			// static std::string tid;
// 			// if(g_fp == nullptr) {
// 			// 	char szPath[MAX_PATH] = { };
// 			// 	GetModuleFileNameA(NULL, szPath, MAX_PATH);
// 			// 	char *p = NULL;
// 			// 	p = strrchr(szPath, '\\');
// 			// 	modulename = p + 1;
// 			// 	tid = std::to_string(GetCurrentThreadId());
// 			// 	std::string filename = std::string("C:\\Users\\Ceylon\\Desktop\\ReverseKits\\hook\\") + modulename + tid + ".txt";
// 			// 	g_fp = fopen(filename.c_str(), "a");
// 			// }
// 			// std::string content;
// 			// content = std::string("wParam: ") + std::to_string(wParam) + "  module: " + modulename + "  thread: " + tid + "  "
// 			// 			+ "ncode: " + std::to_string(code) + "  vkcode: " + std::to_string(vk->vkCode) + "\n";
// 			// fputs(content.c_str(), g_fp);

// 		}
// 	}
// 	return CallNextHookEx(g_hHook, code, wParam, lParam);
// }

#ifdef __cplusplus
extern "C" {
#endif
	// __declspec(dllexport) 
    auto HookStart() -> decltype(g_hHook)
	{
		g_hHook = SetWindowsHookExA(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);
		// g_hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProcLL, g_hInstance, 4052);
		return g_hHook;
	}

	// __declspec(dllexport) 
    void HookStop()
	{
		if( g_hHook )
		{
			UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
#ifdef __cplusplus
}
#endif
#include "header.h"

LPVOID g_messagebox;
CREATE_PROCESS_DEBUG_INFO g_dbginit;
unsigned char g_OrgByte;
unsigned char g_OrgByte2;
const unsigned char INT3 = 0xCC;

void OnCreateProcessDebugEvent(LPDEBUG_EVENT pde) {
    g_messagebox = (LPVOID)MessageBoxA;
    memcpy(&g_dbginit, &pde->u.CreateProcessInfo, sizeof(CREATE_PROCESS_DEBUG_INFO));
    ReadProcessMemory(g_dbginit.hProcess, g_messagebox, &g_OrgByte, sizeof(unsigned char), nullptr);
    ReadProcessMemory(g_dbginit.hProcess, (LPCVOID)((size_t)g_messagebox + 4), &g_OrgByte2, sizeof(unsigned char), nullptr);
    WriteProcessMemory(g_dbginit.hProcess, g_messagebox, &INT3, sizeof(unsigned char), nullptr);
}

void OnExceptionDebugEvent(LPDEBUG_EVENT pde) { 
    PEXCEPTION_RECORD per = &pde->u.Exception.ExceptionRecord;
    if(per->ExceptionCode != EXCEPTION_BREAKPOINT) {
        cout << " per->ExceptionCode != EXCEPTION_BREAKPOINT" << endl;
        return;
    }
    if(per->ExceptionAddress == (LPVOID)((size_t)g_messagebox + 4)) {
        WriteProcessMemory(g_dbginit.hProcess, g_messagebox, &INT3, sizeof(unsigned char), nullptr);
        WriteProcessMemory(g_dbginit.hProcess, (LPVOID)((size_t)g_messagebox + 4), &g_OrgByte2, sizeof(unsigned char), nullptr);
        CONTEXT ctx = { };
        ctx.ContextFlags = CONTEXT_ALL;
        GetThreadContext(g_dbginit.hThread, &ctx);
        ctx.Rip--;
        SetThreadContext(g_dbginit.hThread, &ctx);
        return;
    }
    if(per->ExceptionAddress != g_messagebox) {
        cout << " per->ExceptionAddress: " << per->ExceptionAddress << endl;
        return;
    }
    CONTEXT ctx = { };
    WriteProcessMemory(g_dbginit.hProcess, g_messagebox, &g_OrgByte, sizeof(unsigned char), nullptr);
    WriteProcessMemory(g_dbginit.hProcess, (LPVOID)((size_t)g_messagebox + 4), &INT3, sizeof(unsigned char), nullptr);
    ctx.ContextFlags = CONTEXT_ALL;
    GetThreadContext(g_dbginit.hThread, &ctx);
    auto lpBuffer = new unsigned char[4]{ };
    ReadProcessMemory(g_dbginit.hProcess, (LPCVOID)ctx.Rdx, lpBuffer, 3, NULL);
    for(int i = 0; i < 3; i++ ) {
        if(0x61 <= lpBuffer[i] && lpBuffer[i] <= 0x7A)
            lpBuffer[i] -= 0x20;
    }
    static unsigned char c = 0x30;
    lpBuffer[2] = c++;
    DWORD oldprotect;
    VirtualProtectEx(g_dbginit.hProcess, (LPVOID)ctx.Rdx, 3, PAGE_EXECUTE_READWRITE, &oldprotect);
    WriteProcessMemory(g_dbginit.hProcess, (LPVOID)ctx.Rdx, lpBuffer, 3, nullptr);
    VirtualProtectEx(g_dbginit.hProcess, (LPVOID)ctx.Rdx, 3, oldprotect, &oldprotect);
    delete[] lpBuffer;
    ctx.Rip--;
    SetThreadContext(g_dbginit.hThread, &ctx);
}

void DebugLoop() {
    DEBUG_EVENT de;

    while(WaitForDebugEvent(&de, INFINITE)) {
        if(de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
            cout << "on init" << endl;
            OnCreateProcessDebugEvent(&de);
        } else if(de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            OnExceptionDebugEvent(&de);
        } else if(de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
            break;
        }
        ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
    }
}

int main() {
    DWORD pid;
    cout << "input pid: " << endl;
    cin >> pid;
    DebugActiveProcess(pid);
    DebugLoop();

    return 0;
}


// #include "header.h"

// LPVOID g_messagebox;
// CREATE_PROCESS_DEBUG_INFO g_dbginit;
// unsigned char g_OrgByte;
// const unsigned char INT3 = 0xCC;

// void OnCreateProcessDebugEvent(LPDEBUG_EVENT pde) {    
//     memcpy(&g_dbginit, &pde->u.CreateProcessInfo, sizeof(CREATE_PROCESS_DEBUG_INFO));
//     // g_messagebox = (LPVOID)GetProcAddress(GetModuleHandleA("user32.dll"), "MessageBoxA");
//     g_messagebox = (LPVOID)MessageBoxA;
//     ReadProcessMemory(g_dbginit.hProcess, g_messagebox, &g_OrgByte, sizeof(unsigned char), nullptr);
//     WriteProcessMemory(g_dbginit.hProcess, g_messagebox, &INT3, sizeof(unsigned char), nullptr);
// }

// void OnExceptionDebugEvent(LPDEBUG_EVENT pde) { 
//     PEXCEPTION_RECORD per = &pde->u.Exception.ExceptionRecord;
//     if(per->ExceptionCode != EXCEPTION_BREAKPOINT) {
//         cout << " per->ExceptionCode != EXCEPTION_BREAKPOINT" << endl;
//         return;
//     }
//     if(per->ExceptionAddress != g_messagebox) {
//         cout << " per->ExceptionAddress: " << per->ExceptionAddress << endl;
//         return;
//     }
//     WriteProcessMemory(g_dbginit.hProcess, g_messagebox, &g_OrgByte, sizeof(unsigned char), nullptr);
//     CONTEXT ctx = { };
//     ctx.ContextFlags = CONTEXT_ALL;
//     auto res = GetThreadContext(g_dbginit.hThread, &ctx);
//     cout << (void*)ctx.Rip << "  " << ctx.ContextFlags << "  " << ctx.SegCs << endl;
//     cout << "Rdx: " << (void*)ctx.Rdx << "  " << res << endl;
//     auto tmp = new unsigned char [4] { };
//     ReadProcessMemory(g_dbginit.hProcess, (LPCVOID)ctx.Rdx, tmp, 3, nullptr);
//     cout << "stemp: " << (char*)tmp << endl;
//     delete[] tmp;
//     auto lpBuffer = new unsigned char[3]{ };
//     memset(lpBuffer, 0, 3);
//     ReadProcessMemory(g_dbginit.hProcess, (LPCVOID)ctx.Rdx, lpBuffer, 2, nullptr);
//     for(int i = 0; i < 2; i++ ) {
//         if(0x61 <= lpBuffer[i] && lpBuffer[i] <= 0x7A)
//             lpBuffer[i] -= 0x20;
//     }
//     WriteProcessMemory(g_dbginit.hProcess, (LPVOID)ctx.Rdx, lpBuffer, 3, nullptr);
//     delete[] lpBuffer;
//     ctx.Rip = (DWORD64)g_messagebox;
//     SetThreadContext(g_dbginit.hThread, &ctx);
// }


// DWORD pid;

// void DebugLoop() {
//     DEBUG_EVENT de;
//     bool flag = false;
//     while(WaitForDebugEvent(&de, INFINITE)) {
//         if(flag) {
//             cout <<"in debug..." << endl;
//         }
//         if(de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
//             cout << "on init" << endl;
//             flag = true;
//             OnCreateProcessDebugEvent(&de);
//         } else if(de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
//             OnExceptionDebugEvent(&de);
//         } else if(de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
//             break;
//         }
//         ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
//     }
// }

// int main() {
//     cin >> pid;
//     DebugActiveProcess(pid);
//     DebugLoop();
//     return 0;
// }
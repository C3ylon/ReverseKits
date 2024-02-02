#include "header.h"
#include "fundamental.h"

HMODULE dllmod;
HMODULE exemod;
HANDLE hProcess;

void GetModAddr(DWORD pid) {
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    bool more = Module32First(hSnapshot, &me);
    for( ; more; more = Module32Next(hSnapshot, &me)) {
        if(string("dll.dll") == string(me.szModule)) {
            dllmod = me.hModule;
        }
        if(string("test.exe") == string(me.szModule)) {
            exemod = me.hModule;
        }
    }
    CloseHandle(hSnapshot);
}

void *GetIAT() {
    void *iat = nullptr;
    DWORD ntoffset;
    clre::ReadMemory(hProcess, (void*)((size_t)exemod + 0x3c), &ntoffset, sizeof ntoffset);
    (size_t &)iat = (size_t)exemod + ntoffset + 0x80;
    return iat;
}

int main() {
    DWORD pid;
    cin >> pid;
    try {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
        clre::InjectDll(pid, R"(.\dll.dll)");
        GetModAddr(pid);
        auto func = GetProcAddress(dllmod, "MyMessageBox");
        auto iat = GetIAT();
        clre::WriteMemory(hProcess, iat, (void*)&func, sizeof(void *));
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}
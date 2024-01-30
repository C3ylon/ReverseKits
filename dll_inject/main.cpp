#include "header.h"

const char dllpath[] = "C:\\Users\\Ceylon\\Desktop\\ReverseKits\\dll_inject\\testdll.dll";


void InjectDll(DWORD pid, const char *dllpath) {
    const auto pathsize = string(dllpath).length() + 1; 
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if(hProcess == nullptr)
        throw std::runtime_error("OpenProcess failed");
    auto pRemotebuffer = VirtualAllocEx(hProcess, nullptr, pathsize, MEM_COMMIT, PAGE_READWRITE);
    cout << "remote buffer address: " << pRemotebuffer << endl;
    WriteProcessMemory(hProcess, pRemotebuffer, dllpath, pathsize, nullptr);
    auto hMod = GetModuleHandleA("kernel32.dll");
    auto thread_proc = GetProcAddress(hMod, "LoadLibraryA");
    if(thread_proc == nullptr) {
        CloseHandle(hProcess);
        throw std::runtime_error("GetProcAddress LoadLibraryA failed");
    }
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, *(LPTHREAD_START_ROUTINE *)&thread_proc, pRemotebuffer, 0, nullptr);
    if(hThread == nullptr) {
        CloseHandle(hProcess);
        throw std::runtime_error("CreateRemoteThread failed");
    }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

void EjectDll(DWORD pid, const char *dllname) {
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    bool found = false;
    bool more = Module32First(hSnapshot, &me);
    for( ; more; more = Module32Next(hSnapshot, &me)) {
        if(string(me.szModule) == string(dllname)
            || string(me.szExePath) == string(dllname)) {
                FILE *fp = fopen(R"(C:\Users\Ceylon\Desktop\ReverseKits\dll_inject\log.txt)", "a");
                string res = string(me.szModule) + "\t" + me.szExePath + "\n";
                fputs(res.c_str(), fp);
                fclose(fp);
                found = true;
                break;
            }
    }
    CloseHandle(hSnapshot);
    if(found == false)
        throw std::runtime_error("Find Module Entry failed");
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if(hProcess == nullptr)
        throw std::runtime_error("OpenProcess falied");
    auto hMod = GetModuleHandleA("kernel32.dll");
    auto thread_proc = GetProcAddress(hMod, "FreeLibrary");
    auto hThread = CreateRemoteThread(hProcess, nullptr, 0, *(LPTHREAD_START_ROUTINE *)&thread_proc, me.modBaseAddr, 0, nullptr);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

int main() {
    cout << "input pid: " << endl;
    DWORD pid;
    cin >> pid;
    while(1) {
        cout << "1. inject dll    2.eject dll" << endl;
        int op;
        cin >> op;
        try {
            if(op == 1) {
                InjectDll(pid, dllpath);
            } else if(op == 2) {
                EjectDll(pid, "testdll.dll");
            } else {
                break;
            }
        } catch (std::exception &e) {
            cout << e.what() << endl;
        }
    }
    return 0;
}
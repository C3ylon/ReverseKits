#include "memory.h"

unsigned char buff[3] = { 0 };
unsigned char bullet[3] = { 0x90, 0x90, 0x90 };

int main() {
    HMODULE hmod;
    if(InitByProcessNameW(L"tlou-i.exe")) {
        hmod = GetModuleBaseAddr(L"tlou-i.exe");
        LOG_INFO("hmod : %p", hmod);
        UINT64 addr = ScanforSignature("83 E8 01 66 0F 49 C8 66", (UINT64)hmod, 0, 0);
        LOG_INFO("addr : %I64X", addr);
        if(addr) {
            ReadMemory((SIZE_T)addr, 3, buff);
            LOG_INFO("%02X %02X %02X", buff[0], buff[1], buff[2]);
            WriteMemory((SIZE_T)addr, 3, bullet);

            getchar();
            WriteMemory((SIZE_T)addr, 3, buff);

            CloseHandle(hProcess);
        }
    } else {
        LOG_ERROR("can't find process");
    }
    return 0;
}
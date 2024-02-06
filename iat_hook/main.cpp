#include "header.h"
#include "fundamental.h"

void *GetFuncInIAT64(HANDLE hProcess, void *func_va) {
    auto image_base = clre::GetImageBase(hProcess);
    auto rva_to_va = [image_base](DWORD rva) { return (void*)((size_t)image_base + rva); };
    DWORD ntoffset;
    clre::ReadMemory(hProcess, rva_to_va(0x3C), &ntoffset, sizeof(DWORD));
    DWORD iat_rva;
    clre::ReadMemory(hProcess, rva_to_va(ntoffset + 0x90), &iat_rva, sizeof(DWORD));
    IMAGE_IMPORT_DESCRIPTOR iid;
    static const IMAGE_IMPORT_DESCRIPTOR iid_zero_end = { };
    auto iid_va = rva_to_va(iat_rva);
    while(true) {
        clre::ReadMemory(hProcess, iid_va, &iid, sizeof(iid));
        if(memcmp(&iid, &iid_zero_end, sizeof(iid)) == 0)
            break;
        auto addr_table = (void**)rva_to_va(iid.FirstThunk);
        for(int i = 0; ; i++) {
            void *res;
            clre::ReadMemory(hProcess, addr_table + i, &res, sizeof(void*));
            if(res == nullptr)
                break;
            if(res == func_va)
                return addr_table + i;
        }
        (size_t&)iid_va += sizeof(iid);
    }
    throw std::runtime_error("Can't find func VA in IAT");
}


int main() {
    DWORD pid;
    HANDLE hProcess;
    cin >> pid;
    try {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
        clre::InjectDll(hProcess, R"(C:\Users\Ceylon\Desktop\ReverseKits\iat_hook\dll.dll)");
        auto myfunc = clre::GetProcAddressEx64(hProcess, clre::GetModInfo(pid, "dll.dll").hModule, "MyMessageBox");
        auto original_func_in_iat = GetFuncInIAT64(hProcess, (void*)(size_t)MessageBoxA);
        cout << "myfunc : " << myfunc << "\toriginal_func_in_iat : " << original_func_in_iat << endl;
        void *original_func;
        clre::ReadMemory(hProcess, original_func_in_iat, &original_func, sizeof(void*));
        clre::WriteMemory(hProcess, original_func_in_iat, &myfunc, sizeof(void*));
        int c;
        cin >> c;
        clre::WriteMemory(hProcess, original_func_in_iat, &original_func, sizeof(void*));
        clre::EjectDll(hProcess, pid, "dll.dll");
    } catch (std::exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}
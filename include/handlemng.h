#ifndef HANDLEMNG__H_
#define HANDLEMNG__H_

#include "header.h"

namespace clre {

class HandleMng {
    HANDLE h;
    void close(HANDLE &handle) { if(handle != nullptr) CloseHandle(handle); handle = nullptr; }
    public:
    HandleMng(HANDLE handle) : h(handle) { }
    HandleMng(HandleMng &&m) {
        h = m.h;
        m.h = nullptr;
    }
    HandleMng &operator=(HandleMng &&m) {
        auto tmp = m.h;
        m.h = nullptr;
        if(h != tmp) {
            close(h);
            h = tmp;
        }
        return *this;
    }
    ~HandleMng() { close(h); }

};


}

#endif
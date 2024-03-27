#ifndef FILEMNG__H_
#define FILEMNG__H_
#include <cstdio>
#include <stdexcept>
#include <string>

namespace clre {

template <typename Fnclose = decltype(&fclose),
          Fnclose _close = &fclose,
          typename Fnopen = decltype(&fopen),
          Fnopen _open = &fopen>
class filemng {
    std::FILE *fp = nullptr;
public:
    filemng() = default;
    filemng(FILE *fp) : fp(fp) { 
        if(fp == nullptr)
            throw std::runtime_error(std::string("[!]fp is a nullptr"));
    }
    filemng(const char *path, const char *mod) {
        fp = _open(path, mod);
        if(fp == nullptr)
            throw std::runtime_error(std::string("[!]Open file: ") + path + "fail");
    }
    filemng(const std::string &path, const char *mod) :
        filemng(path.c_str(), mod) {
    }
    ~filemng() { if(fp != nullptr) _close(fp); }

    filemng(const filemng&) = delete;
    filemng &operator =(const filemng&) = delete;

    filemng &operator =(FILE *const fp) {
        if(fp == nullptr) {
            throw std::runtime_error("[!]fp is a nullptr");
        }
        if(filemng::fp == fp)
            return *this;
        if(filemng::fp != nullptr)
            _close(filemng::fp);
        filemng::fp = fp;
        return *this;
    }
    filemng(filemng &&f) noexcept : fp(f.fp) {
        f.fp = nullptr;
    }
    filemng &operator =(filemng &&f) noexcept {
        if(&f == this)
            return *this;
        if(fp == f.fp) {
            f.fp = nullptr;
            return *this;
        }
        if(fp != nullptr)
            _close(fp);
        fp = f.fp;
        f.fp = nullptr;
        return *this;
    }
    explicit operator bool() const noexcept { return fp != nullptr; }
    operator FILE*() const noexcept { return fp; }
    FILE *get() const noexcept { return fp; }
};

using FileMng = filemng<>;

}

#endif
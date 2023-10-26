#ifndef FILEMNG__H_
#define FILEMNG__H_
#include <cstdio>

namespace clre {

template <typename Fnclose = decltype(&fclose),
          Fnclose close = &fclose,
          typename Fnopen = decltype(&fopen),
          Fnopen open = &fopen>
class filemng {
    std::FILE *fp = nullptr;
public:
    filemng() = default;
    explicit filemng(FILE *fp) noexcept : fp(fp) { }
    filemng(const char*path, const char*mod) { fp = open(path, mod); }
    ~filemng() { if(fp != nullptr) fclose(fp); }

    filemng(const filemng&) = delete;
    filemng &operator =(const filemng&) =delete;

    filemng &operator =(FILE *const fp) noexcept {
        if(filemng::fp == fp)
            return *this;
        if(filemng::fp != nullptr)
            close(filemng::fp);
        filemng::fp = fp;
        return *this;
    }
    filemng(filemng &&f) noexcept : fp(f.fp) {
        f.fp = nullptr;
    }
    filemng &operator =(filemng &&f) noexcept {
        if(&f == this)
            return *this;
        if(fp!= nullptr)
            close(fp);
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
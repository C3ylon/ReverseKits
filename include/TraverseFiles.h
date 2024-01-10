#ifndef TRAVERSE_FILES__H_
#define TRAVERSE_FILES__H_

#include <sys/stat.h>
#include <stdexcept>

namespace clre{

template <typename T, typename ...Args>
void TraversalFiles(const char *, T, Args...);

template <typename T, typename ...Args>
void TraversalSubdir(const __finddata64_t &FindData,
                     const char *dir,
                     T FileOp,
                     Args ...argc) {
    bool is_sub_dir = FindData.attrib & _A_SUBDIR;
    if(is_sub_dir == true) {
        auto foldername = [&](const char *s) {
            return std::string(FindData.name) == s;
        };
        if (foldername(".") || foldername("..")) {
            return;
        }
        std::cout << "[FOLDER]" << FindData.name << "\n";
        std::string dirNew = std::string(dir) + "\\" + FindData.name;
        TraversalFiles(dirNew.c_str(), FileOp, argc...);
        return;
    }
    std::cout << "[FILE]" << FindData.name << "\t" << FindData.size << " bytes";
    try {
        clock_t start, end;
        start = clock();
        std::string fullpath = std::string(dir) + "\\" + FindData.name;
        FileOp(fullpath.c_str(), argc...);
        end = clock();
        std::cout << "\t" "time: " << (double)(end - start) / CLOCKS_PER_SEC << "s\n";
    } catch (const std::exception &e) {
        std::cout << e.what() << "\n";
    }
}

template <typename T, typename ...Args>
void TraversalFolder(const char *dir, T FileOp, Args ...argc) {
    std::string dirNew = std::string(dir) + "\\*.*";
    struct __finddata64_t FindData;
    intptr_t handle = _findfirst64(dirNew.c_str(), &FindData);
    if (handle == -1) {
        throw std::runtime_error("[!]find path fail");
    }
    do {
        TraversalSubdir(FindData, dir, FileOp, argc...);
    } while(_findnext64(handle, &FindData) == 0);
    _findclose(handle);
}

template <typename T, typename ...Args>
void TraversalFiles(const char *dir, T FileOp, Args ...argc) {
    struct _stat64 sbuff;
    if(_stat64(dir, &sbuff) == -1) {
        throw std::runtime_error("[!]find file or folder path fail");
    }
    bool is_reg = sbuff.st_mode & _S_IFREG;
    bool is_dir = sbuff.st_mode & S_IFDIR;
    // input path is a sigle file
    if(is_reg == true) {
        std::cout << "[*]file size: " << sbuff.st_size << "bytes\n";
        FileOp(dir, argc...);
    }
    // input path is a folder
    else if(is_dir == true) {
        TraversalFolder(dir, FileOp, argc...);
    }
}

}

#endif

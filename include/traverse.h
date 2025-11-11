#include "Encoding.h"

namespace clre {

template <typename FuncFileOp, typename FuncFolderOp>
void traverseFolder(const std::wstring &directory, FuncFileOp &&fileOp, FuncFolderOp &&folderOp) {
    WIN32_FIND_DATAW findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    std::wstring searchPath = directory + L"\\*";

    hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        const std::wstring itemName = findData.cFileName;
        if (itemName == L"." || itemName == L"..")
            continue;
        std::wstring fullPath = directory + L"\\" + itemName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            folderOp(fullPath);
        } else {
            fileOp(fullPath);
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

template <typename FuncFileOp, typename FuncFolderOp>
void traverseAllFiles(const std::wstring &directory, FuncFileOp &&fileOp, FuncFolderOp &&folderOp) {
    class Functor {
        FuncFileOp &fileOp;
        FuncFolderOp &folderOp;
    public:
        Functor(FuncFileOp &fileOp, FuncFolderOp &folderOp) : fileOp(fileOp), folderOp(folderOp) { }
        void operator()(const std::wstring path) const {
            folderOp(path);
            traverseFolder(path, fileOp, *this);
        }
    };

    traverseFolder(directory, fileOp, Functor(fileOp, folderOp));
}



}
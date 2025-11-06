#include "Encoding.h"

namespace clre {

template <typename Func>
void traverseDirectory(const std::wstring &directory, Func &&fileHandler)
{
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
            traverseDirectory(fullPath, std::forward<Func>(fileHandler));
        } else {
            fileHandler(fullPath);
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

}
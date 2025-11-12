#ifndef ENCODING__H_
#define ENCODING__H_

#include <string>
#include <vector>
#include <windows.h>
#include <type_traits>

namespace clre {

inline std::wstring utf8ToWstring(const std::string &utf8Str) {
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (sizeNeeded <= 0) {
        return L"";
    }
    std::wstring wstr(sizeNeeded - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], sizeNeeded);
    return wstr;
}

inline std::string wstringToUtf8(const std::wstring &wstr) {
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        return "";
    }
    std::string utf8Str(sizeNeeded - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], sizeNeeded, nullptr, nullptr);
    return utf8Str;
}

inline std::string wstringToGbk(const std::wstring &wstr) {
    int sizeNeeded = WideCharToMultiByte(936, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        return "";
    }
    std::string gbk(sizeNeeded - 1, 0);
    WideCharToMultiByte(936, 0, wstr.c_str(), -1, &gbk[0], sizeNeeded, nullptr, nullptr);
    return gbk;
}

class ConsoleIoMng {
public:
    ConsoleIoMng() {
        hIn = GetStdHandle(STD_INPUT_HANDLE);
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    ~ConsoleIoMng() = default;

    static ConsoleIoMng &instance() {
        static ConsoleIoMng obj;
        return obj;
    }

    ConsoleIoMng &operator >>(std::string &s) {
        std::wstring wStr = readWideString();
        s = wstringToUtf8(wStr);
        return *this;
    }

    ConsoleIoMng &operator <<(const std::wstring &s) {
        writeWideString(s);
        return *this;
    }

    ConsoleIoMng &operator <<(const std::string &s) {
        std::wstring wStr = utf8ToWstring(s);
        writeWideString(wStr);
        return *this;
    }

    ConsoleIoMng &operator <<(ConsoleIoMng &(*pEndl)(ConsoleIoMng &)) {
        return pEndl(*this);
    }

    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, ConsoleIoMng &>::value operator <<(T val) {
        std::wstring wStr = std::to_wstring(val);
        writeWideString(wStr);
        return *this;
    }

private:
    void writeWideString(const std::wstring& str) {
        DWORD charsWritten;
        WriteConsoleW(hOut, str.c_str(), str.length(), &charsWritten, NULL);
    }

    std::wstring readWideString() {
        constexpr int MAX_WCHARS = 1024;
        std::vector<wchar_t> buffer(MAX_WCHARS);
        DWORD charsRead = 0;
        if (!ReadConsoleW(hIn, &buffer[0], MAX_WCHARS - 1, &charsRead, NULL)) {
            return L"";
        }
        buffer[charsRead] = L'\0';
        if (charsRead >= 2 && buffer[charsRead - 2] == L'\r' && buffer[charsRead - 1] == L'\n') {
            charsRead -= 2;
        } else if (charsRead >= 1 && (buffer[charsRead - 1] == L'\r' || buffer[charsRead - 1] == L'\n')) {
            charsRead -= 1;
        }
        return std::wstring(buffer.begin(), buffer.begin() + charsRead);
    }

private:
    HANDLE hIn;
    HANDLE hOut;
};

inline ConsoleIoMng &endl(ConsoleIoMng &outIo) {
    outIo << L"\n";
    return outIo;
}

}

#define conio clre::ConsoleIoMng::instance()

#endif
#ifndef ENCODING__H_
#define ENCODING__H_

#include <string>
#include <vector>
#include <windows.h>
#include <type_traits>

namespace clre {

inline std::wstring utf8ToWstring(const std::string &utf8Str) {
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    std::wstring wstr(sizeNeeded - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], sizeNeeded);
    return wstr;
}

inline std::string wstringToUtf8(const std::wstring &wstr) {
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Str(sizeNeeded - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], sizeNeeded, nullptr, nullptr);
    return utf8Str;
}

inline std::string wstringToGbk(const std::wstring &wstr) {
    int sizeNeeded = WideCharToMultiByte(936, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string gbk(sizeNeeded - 1, 0);
    WideCharToMultiByte(936, 0, wstr.c_str(), -1, &gbk[0], sizeNeeded, nullptr, nullptr);
    return gbk;
}

namespace {
// 辅助函数：使用 WriteConsoleW 输出宽字符串
void writeWideString(HANDLE hOut, const std::wstring& str) {
    DWORD chars_written;
    WriteConsoleW(hOut, str.c_str(), str.length(), &chars_written, NULL);
}

// 辅助函数：使用 ReadConsoleW 读取宽字符串
std::wstring readWideString(HANDLE hIn) {
    // 假设输入缓冲区最大为 1024 宽字符
    const int MAX_WCHARS = 1024;
    std::vector<wchar_t> buffer(MAX_WCHARS);
    DWORD chars_read = 0;
    // 使用 ReadConsoleW 读取输入
    if (ReadConsoleW(hIn, &buffer[0], MAX_WCHARS - 1, &chars_read, NULL)) {
        // 确保字符串正确终止
        buffer[chars_read] = L'\0';

        // ReadConsoleW 通常会包含用户按下的回车换行符（\r\n），需要移除
        if (chars_read >= 2 && buffer[chars_read - 2] == L'\r' && buffer[chars_read - 1] == L'\n') {
            chars_read -= 2;
        } else if (chars_read >= 1 && (buffer[chars_read - 1] == L'\r' || buffer[chars_read - 1] == L'\n')) {
            chars_read -= 1;
        }

        return std::wstring(buffer.begin(), buffer.begin() + chars_read);
    }
    return L""; // 读取失败
}
}

class ConsoleIoMng {
public:
    ConsoleIoMng() {
        hIn = GetStdHandle(STD_INPUT_HANDLE);
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    ~ConsoleIoMng() {
        CloseHandle(hIn);
        CloseHandle(hOut);
    }

    ConsoleIoMng &operator >>(std::string &s) {
        std::wstring wStr = readWideString(hIn);
        s = wstringToUtf8(wStr);
        return *this;
    }

    ConsoleIoMng &operator <<(const std::wstring &s) {
        writeWideString(hOut, s);
        return *this;
    }

    ConsoleIoMng &operator <<(const std::string &s) {
        std::wstring wStr = utf8ToWstring(s);
        writeWideString(hOut, wStr);
        return *this;
    }

    template<typename T>
    std::enable_if_t<std::is_integral_v<T>, ConsoleIoMng &> operator <<(T val) {
        std::wstring wStr = std::to_wstring(val);
        writeWideString(hOut, wStr);
        return *this;
    }

    template<typename T>
    std::enable_if_t<std::is_floating_point_v<T>, ConsoleIoMng &> operator <<(T val) {
        std::wstring wStr = std::to_wstring(val);
        writeWideString(hOut, wStr);
        return *this;
    }

private:
    HANDLE hIn;
    HANDLE hOut;
};

}



#endif
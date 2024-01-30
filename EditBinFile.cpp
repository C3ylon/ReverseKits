#include "filemng.h"
#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

auto sbyte2byte(const string &sbyte) -> unsigned char {
    unsigned char res = 0;
    for(auto i : sbyte) {
        res <<= 4;
        if('0' <= i && i <= '9') {    
            res += i - '0';
        } else if('a' <= i && i <= 'f') {
            res += i - 'a' + 0xa;
        } else if('A' <= i && i <= 'F') {
            res += i - 'A' + 0xa;
        }
    }
    return res;
}

int main() {
    // cout << "input file path: " << endl;
    string filepath;
    // cin >> filepath;
    filepath = R"(C:\Users\Ceylon\Desktop\TextView.exe)";
    auto fp = clre::FileMng(filepath, "rb+");
    cout << "RAW\tval" << endl;
    size_t raw;
    cin >> raw;
    string sbyte;
    vector<unsigned char> bytes;
    while(cin >> sbyte) {
        bytes.push_back(sbyte2byte(sbyte));
    }
    _fseeki64(fp, raw, SEEK_SET);
    auto p = new unsigned char[bytes.size()];
    for(size_t i = 0; i < bytes.size(); i++) {
        p[i] = bytes[i];
    }
    fwrite(p, bytes.size(), 1, fp);
    delete[] p;
    return 0;
}


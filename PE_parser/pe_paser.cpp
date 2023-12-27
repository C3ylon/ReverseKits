#include <iostream>
#include <string>
#include <vector>
#include "filemng.h"

#include <windows.h>

using std::string;
using std::vector;

clre::FileMng fp;
bool is_x64;
vector<string> printbuffer;

string printmemory(void *addr, size_t size) {
    string res;
    char ch[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    for(size_t i = 0; i < size; i++) {
        res += ch[((char*)addr)[i] / 16];
        res += ch[((char*)addr)[i] % 16];
        res += ' ';
    }
    res.pop_back();
    return res;
}

void parse_dos_header() {
    WORD e_magic = 0;
    fread(&e_magic, 2, 1, fp);
    if(e_magic != 0x5A4D) {
        throw string("[!]dos header magic error: ") + printmemory(&e_magic, 2);
    }
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    try {
        if(argc < 2) {
            throw string("[!]Missing parameter: file_path");
        }
        fp = clre::FileMng(argv[1], "rb");
        parse_dos_header();
    } catch (const string &e) {
        std::cout << e << "\n";
    }
    std::cout << std::flush;
    system("pause");
    return 0;
}
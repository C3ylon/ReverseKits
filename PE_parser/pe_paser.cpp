#include <iostream>
#include <string>
#include <vector>

#include <windows.h>

#include "filemng.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

clre::FileMng fp;
bool is_x64;
vector<string> printbuffer;

string printmemory(void *addr, size_t size) {
    string res;
    char ch[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    for(size_t i = 0; i < size; i++) {
        res += ch[((unsigned char*)addr)[i] / 16];
        res += ch[((unsigned char*)addr)[i] % 16];
        if(i % 16 == 15)
            res += '\n';
        else
            res += ' ';
    }
    res.pop_back();
    return res;
}

void output() {
    auto size = printbuffer.size();
    for(decltype(size) i = 0; i < size; i++) {
        std::cout << printbuffer[i] << "\n";
    }
}

void parse_dos_header(DWORD &e_lfanew) {
    WORD e_magic = 0;
    fread(&e_magic, 2, 1, fp);
    if(e_magic != 0x5A4D) {
        throw string("[!]dos header magic error: ") + printmemory(&e_magic, 2);
    }
    printbuffer.push_back("[*]DOS header:");
    _fseeki64(fp, 0x3C, 0);
    fread(&e_lfanew, 4, 1, fp);
    printbuffer.push_back(string("e_magic: ") + printmemory(&e_magic, 2));
    printbuffer.push_back(string("e_lfanew: ") + printmemory(&e_lfanew, 4));
}

void parse_file_header() {
    IMAGE_FILE_HEADER fileheader;
    fread(&fileheader, sizeof(IMAGE_FILE_HEADER), 1, fp);
    printbuffer.push_back(string(30, '-'));
    printbuffer.push_back(string("Machine: ") + printmemory(&fileheader.Machine, 2));
    printbuffer.push_back(string("NumberOfSections: ") + printmemory(&fileheader.NumberOfSections, 2));
    printbuffer.push_back(string("SizeOfOptionalHeader: ") + printmemory(&fileheader.SizeOfOptionalHeader, 2));
    printbuffer.push_back(string("Characteristics: ") + printmemory(&fileheader.Characteristics, 2));
}

void parse_nt_header(DWORD e_lfanew) {
    printbuffer.push_back(string(30, '='));
    DWORD Signature = 0;
    _fseeki64(fp, e_lfanew, 0);
    fread(&Signature, 4, 1, fp);
    if(Signature != 0x4550) {
        throw string("[!]nt header magic error: ") + printmemory(&Signature, 4);
    }
    printbuffer.push_back("[*]NT header:");
    printbuffer.push_back(string("Signature: ") + printmemory(&Signature, 4));
    parse_file_header();
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    try {
        if(argc < 2) {
            throw string("[!]Missing parameter: file_path");
        }
        fp = clre::FileMng(argv[1], "rb");
        DWORD e_lfanew = 0;
        parse_dos_header(e_lfanew);
        parse_nt_header(e_lfanew);
        output();
    } catch (const string &e) {
        std::cout << e << "\n";
    }
    std::cout << std::flush;
    system("pause");
    return 0;
}
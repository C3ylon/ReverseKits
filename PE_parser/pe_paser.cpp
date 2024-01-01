#include <iostream>
#include <string>
#include <vector>

#include <windows.h>

#include "filemng.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

const size_t common_optional_header_lenth
    = (size_t)&((IMAGE_OPTIONAL_HEADER64*)(0))->SizeOfStackReserve;
const size_t x64_offset_in_opheader
    = (size_t)&((IMAGE_OPTIONAL_HEADER64*)(0))->NumberOfRvaAndSizes
    - (size_t)&((IMAGE_OPTIONAL_HEADER64*)(0))->SizeOfStackReserve;
const size_t x32_offset_in_opheader
    = (size_t)&((IMAGE_OPTIONAL_HEADER32*)(0))->NumberOfRvaAndSizes
    - (size_t)&((IMAGE_OPTIONAL_HEADER32*)(0))->SizeOfStackReserve;

clre::FileMng fp;
vector<string> printbuffer;

bool is_x64;

DWORD e_lfanew;
WORD NumberOfSections;
vector<IMAGE_SECTION_HEADER> section_header;
IMAGE_DATA_DIRECTORY import_directory;
IMAGE_DATA_DIRECTORY export_directory;

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

void parse_dos_header() {
    printbuffer.push_back("[*]DOS header:");
    WORD e_magic = 0;
    fread(&e_magic, 2, 1, fp);
    if(e_magic != 0x5A4D) {
        throw string("[!]dos header magic error: ") + printmemory(&e_magic, 2);
    }
    _fseeki64(fp, 0x3C, SEEK_SET);
    fread(&e_lfanew, 4, 1, fp);
    printbuffer.push_back(string("e_magic: ") + printmemory(&e_magic, 2));
    printbuffer.push_back(string("e_lfanew: ") + printmemory(&e_lfanew, 4));
}

void parse_file_header() {
    printbuffer.push_back(string(30, '-'));
    IMAGE_FILE_HEADER fileheader;
    fread(&fileheader, sizeof(IMAGE_FILE_HEADER), 1, fp);
    printbuffer.push_back(string("Machine: ") + printmemory(&fileheader.Machine, 2));
    printbuffer.push_back(string("NumberOfSections: ") + printmemory(&fileheader.NumberOfSections, 2));
    NumberOfSections = fileheader.NumberOfSections;
    printbuffer.push_back(string("SizeOfOptionalHeader: ") + printmemory(&fileheader.SizeOfOptionalHeader, 2));
    printbuffer.push_back(string("Characteristics: ") + printmemory(&fileheader.Characteristics, 2));
}

void parse_optional_header() {
    printbuffer.push_back(string(30, '-'));
    IMAGE_OPTIONAL_HEADER64 opheader;
    fread(&opheader, common_optional_header_lenth, 1, fp);
    if(opheader.Magic == 0x020B)
        is_x64 = true;
    else if(opheader.Magic == 0x010B)
        is_x64 = false;
    else
        throw string("[!]optional header magic error: ") + printmemory(&opheader.Magic, 2);
    printbuffer.push_back(string("Magic: ") + printmemory(&opheader.Magic, 2));
    printbuffer.push_back(string("AddressOfEntryPoint: ") + printmemory(&opheader.AddressOfEntryPoint, 4));
    if(is_x64)
        printbuffer.push_back(string("ImageBase: ") + printmemory(&opheader.ImageBase, 8));
    else
        printbuffer.push_back(string("ImageBase: ") + printmemory((char*)&opheader.ImageBase + 4, 4));
    printbuffer.push_back(string("SectionAlignment: ") + printmemory(&opheader.SectionAlignment, 4));
    printbuffer.push_back(string("FileAlignment: ") + printmemory(&opheader.FileAlignment, 4));
    printbuffer.push_back(string("SizeOfImage: ") + printmemory(&opheader.SizeOfImage, 4));
    printbuffer.push_back(string("SizeOfHeaders: ") + printmemory(&opheader.SizeOfHeaders, 4));
    printbuffer.push_back(string("Subsystem: ") + printmemory(&opheader.Subsystem, 2));
    DWORD NumberOfRvaAndSizes;
    if(is_x64)
        _fseeki64(fp, x64_offset_in_opheader, SEEK_CUR);
    else
        _fseeki64(fp, x32_offset_in_opheader, SEEK_CUR);
    fread(&NumberOfRvaAndSizes, 4, 1, fp);
    printbuffer.push_back(string("NumberOfRvaAndSizes: ") + printmemory(&NumberOfRvaAndSizes, 4));
    printbuffer.push_back(string("DataDirectory:"));
    auto datadirectory = new IMAGE_DATA_DIRECTORY[NumberOfRvaAndSizes];
    fread(datadirectory, sizeof(IMAGE_DATA_DIRECTORY), NumberOfRvaAndSizes, fp);
    printbuffer.push_back(printmemory(datadirectory, sizeof(IMAGE_DATA_DIRECTORY)*NumberOfRvaAndSizes));
    memcpy(&import_directory, &datadirectory[1], sizeof(IMAGE_DATA_DIRECTORY));
    memcpy(&export_directory, &datadirectory[0], sizeof(IMAGE_DATA_DIRECTORY));
    delete[] datadirectory;
}

void parse_nt_header() {
    printbuffer.push_back(string(30, '='));
    DWORD Signature = 0;
    _fseeki64(fp, e_lfanew, SEEK_SET);
    fread(&Signature, 4, 1, fp);
    if(Signature != 0x4550) {
        throw string("[!]nt header magic error: ") + printmemory(&Signature, 4);
    }
    printbuffer.push_back("[*]NT header:");
    printbuffer.push_back(string("Signature: ") + printmemory(&Signature, 4));
    parse_file_header();
    parse_optional_header();
}

void parse_section_header() {
    printbuffer.push_back(string(30, '='));
    IMAGE_SECTION_HEADER secheader;
    auto print_section_name = [&]() -> string {
        string res;
        for(int i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++) {
            res += secheader.Name[i];
        }
        return res;
    };
    for(auto i = 0; i < NumberOfSections; i++) {
        printbuffer.push_back(string("[*]section header ") + std::to_string(i+1) + " :");
        fread(&secheader, sizeof(secheader), 1, fp);
        section_header.push_back(secheader);
        printbuffer.push_back(string("Name: ") + printmemory(&secheader.Name, 8) + "  ascii: " + print_section_name());
        printbuffer.push_back(string("VirtualSize: ") + printmemory(&secheader.Misc.VirtualSize, 4));
        printbuffer.push_back(string("VirtualAddress: ") + printmemory(&secheader.VirtualAddress, 4));
        printbuffer.push_back(string("SizeOfRawData: ") + printmemory(&secheader.SizeOfRawData, 4));
        printbuffer.push_back(string("PointerToRawData: ") + printmemory(&secheader.PointerToRawData, 4));
        printbuffer.push_back(string("Characteristics: ") + printmemory(&secheader.Characteristics, 4));
        printbuffer.push_back("");
    }
    printbuffer.pop_back();
}

DWORD rva_to_raw(DWORD rva) {
    DWORD raw = 0;
    for(size_t i = 0; i < section_header.size(); i++) {
        if(rva >= section_header[i].VirtualAddress
            && (i == section_header.size()-1
                || rva < section_header[i+1].VirtualAddress)) {
                    DWORD offset = rva - section_header[i].VirtualAddress;
                    raw = section_header[i].PointerToRawData + offset;
                    return raw;
                }
    }
    throw string("[!]rva to raw wrong");
}

void parse_iat() {
    printbuffer.push_back(string(30, '='));
    DWORD iat_rva = import_directory.VirtualAddress;
    DWORD raw = rva_to_raw(iat_rva);
    DWORD size = import_directory.Size;
    printbuffer.push_back(string("[*]IAT\nraw: ") + printmemory(&raw, 4)+ "\tsize: " + printmemory(&size, 4));
    IMAGE_IMPORT_DESCRIPTOR iid;
    const static IMAGE_IMPORT_DESCRIPTOR iid_zero_end = { };
    _fseeki64(fp, raw, SEEK_SET);
    while(true) {
        fread(&iid, sizeof(IMAGE_IMPORT_DESCRIPTOR), 1, fp);
        auto pos = _ftelli64(fp);
        if(memcmp(&iid, &iid_zero_end, sizeof(IMAGE_IMPORT_DESCRIPTOR)) == 0)
            break;
        printbuffer.push_back(string("OriginalFirstThunk: ") + printmemory(&iid.OriginalFirstThunk, 4));
        
        DWORD name_raw = rva_to_raw(iid.Name);
        _fseeki64(fp, name_raw, SEEK_SET);
        char s[256];
        fread(s, 1, 256, fp);
        _fseeki64(fp, pos, SEEK_SET);
        printbuffer.push_back(string("Name: ") + printmemory(&iid.Name, 4));
        printbuffer.push_back(string("Name raw: ") + printmemory(&name_raw, 4) + "\tcontent: " + s);
        printbuffer.push_back(string("FirstThunk: ") + printmemory(&iid.FirstThunk, 4));
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
        parse_nt_header();
        parse_section_header();
        parse_iat();
        output();
    } catch (const string &e) {
        output();
        std::cout << e << "\n";
    }
    std::cout << std::flush;
    system("pause");
    return 0;
}
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
const size_t size_of_x64 = 8;
const size_t size_of_x32 = 4;

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
    printbuffer.push_back(string(60, '-'));
    IMAGE_FILE_HEADER fileheader;
    fread(&fileheader, sizeof(IMAGE_FILE_HEADER), 1, fp);
    printbuffer.push_back(string("Machine: ") + printmemory(&fileheader.Machine, 2));
    printbuffer.push_back(string("NumberOfSections: ") + printmemory(&fileheader.NumberOfSections, 2));
    NumberOfSections = fileheader.NumberOfSections;
    printbuffer.push_back(string("SizeOfOptionalHeader: ") + printmemory(&fileheader.SizeOfOptionalHeader, 2));
    printbuffer.push_back(string("Characteristics: ") + printmemory(&fileheader.Characteristics, 2));
}

void parse_optional_header() {
    printbuffer.push_back(string(60, '-'));
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
    printbuffer.push_back(string(60, '='));
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
    printbuffer.push_back(string(60, '='));
    IMAGE_SECTION_HEADER secheader;
    auto print_section_name = [&]() -> string {
        string res;
        for(int i = 0; i < IMAGE_SIZEOF_SHORT_NAME && secheader.Name[i]; i++) {
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
    throw string("[!]rva to raw wrong. WRONG RVA: ") + printmemory(&rva, sizeof(rva));
}

void get_func_info(DWORD st_func_rva) {
    DWORD raw = rva_to_raw(st_func_rva);
    _fseeki64(fp, raw, SEEK_SET);
    WORD ordinal;
    fread(&ordinal, sizeof(ordinal), 1, fp);
    printbuffer.push_back(string("rva: ") + printmemory(&st_func_rva, sizeof(st_func_rva))
        + "\traw: " + printmemory(&raw, sizeof(raw))
        + "\tordinal: " + printmemory(&ordinal, sizeof(ordinal)));
    char c;
    string name;
    while(true) {
        fread(&c, 1, 1, fp);
        if(c == 0x00)
            break;
        name += c;
    }
    printbuffer.push_back(string("func name: ") + name);
}

void parse_INT(DWORD raw) {
    _fseeki64(fp, raw, SEEK_SET);
    vector<DWORD> vec_st_func;
    while(true) {
        size_t st_func_rva;
        if(is_x64)
            fread(&st_func_rva, size_of_x64, 1, fp);
        else
            fread(&st_func_rva, size_of_x32, 1, fp);
        if(st_func_rva != 0)
            vec_st_func.push_back(st_func_rva);
        else
            break;
    }
    int count = 1;
    for(auto i:vec_st_func) {
        printbuffer.push_back(string("Number ") + std::to_string(count));
        try {
            get_func_info(i);
        } catch (const string &e) {
            printbuffer.push_back(e);
        }
        count++;
    }
}

void parse_iat() {
    printbuffer.push_back(string(60, '='));
    DWORD iat_rva = import_directory.VirtualAddress;
    DWORD raw = rva_to_raw(iat_rva);
    DWORD size = import_directory.Size;
    printbuffer.push_back(string("[*]IAT\nraw: ") + printmemory(&raw, 4)+ "\tsize: " + printmemory(&size, 4));
    IMAGE_IMPORT_DESCRIPTOR iid;
    const static IMAGE_IMPORT_DESCRIPTOR iid_zero_end = { };
    _fseeki64(fp, raw, SEEK_SET);
    while(true) {
        fread(&iid, sizeof(IMAGE_IMPORT_DESCRIPTOR), 1, fp);
        if(memcmp(&iid, &iid_zero_end, sizeof(IMAGE_IMPORT_DESCRIPTOR)) == 0)
            break;
        auto pos = _ftelli64(fp);
        printbuffer.push_back(string(60, '-'));

        auto print_rva_raw = [](string name, DWORD rva, DWORD raw) {
            printbuffer.push_back(name + " RVA: " + printmemory(&rva, 4)
                + "\tRAW: " + printmemory(&raw, 4));
        };

        DWORD name_raw = rva_to_raw(iid.Name);
        DWORD INT_raw = rva_to_raw(iid.OriginalFirstThunk);
        DWORD IAT_raw = rva_to_raw(iid.FirstThunk); 
        _fseeki64(fp, name_raw, SEEK_SET);
        char s[256];
        fread(s, 1, 256, fp);
        printbuffer.push_back(string("DLL name: ") + s); 
        print_rva_raw("OriginalFirstThunk", iid.OriginalFirstThunk, INT_raw);
        print_rva_raw("Name", iid.Name, name_raw);
        print_rva_raw("FirstThunk", iid.FirstThunk, IAT_raw);

        printbuffer.push_back("\n[*]function imported: ");

        parse_INT(INT_raw);

        _fseeki64(fp, pos, SEEK_SET);
    }
}

void parse_eat() {
    printbuffer.push_back(string(60, '='));
    DWORD eat_rva = export_directory.VirtualAddress;
    if(eat_rva == 0) {
        printbuffer.push_back("[*]Don't have EAT");
        return;
    }
    DWORD raw = rva_to_raw(eat_rva);
    DWORD size = export_directory.Size;
    printbuffer.push_back(string("[*]EAT\nraw: ") + printmemory(&raw, 4)+ "\tsize: " + printmemory(&size, 4));
    IMAGE_EXPORT_DIRECTORY ied;
    _fseeki64(fp, raw, SEEK_SET);
    fread(&ied, sizeof(IMAGE_EXPORT_DIRECTORY), 1, fp);
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
        parse_eat();
        output();
    } catch (const string &e) {
        output();
        std::cout << e << "\n";
    }
    std::cout << std::flush;
    system("pause");
    return 0;
}
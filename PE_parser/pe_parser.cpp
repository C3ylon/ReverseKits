#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

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
decltype(_ftelli64(nullptr)) sec_header_raw;
DWORD FileAlignment;
DWORD SizeOfHeaders;
vector<IMAGE_SECTION_HEADER> section_header;
IMAGE_DATA_DIRECTORY import_directory;
IMAGE_DATA_DIRECTORY export_directory;
IMAGE_DATA_DIRECTORY reloc_directory;

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
        throw std::runtime_error(string("[!]dos header magic error: ") + printmemory(&e_magic, 2));
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
    sec_header_raw = _ftelli64(fp) + fileheader.SizeOfOptionalHeader;
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
        throw std::runtime_error(string("[!]optional header magic error: ") + printmemory(&opheader.Magic, 2));
    printbuffer.push_back(string("Magic: ") + printmemory(&opheader.Magic, 2));
    printbuffer.push_back(string("AddressOfEntryPoint: ") + printmemory(&opheader.AddressOfEntryPoint, 4));
    if(is_x64)
        printbuffer.push_back(string("ImageBase: ") + printmemory(&opheader.ImageBase, 8));
    else
        printbuffer.push_back(string("ImageBase: ") + printmemory((char*)&opheader.ImageBase + 4, 4));
    printbuffer.push_back(string("SectionAlignment: ") + printmemory(&opheader.SectionAlignment, 4));
    printbuffer.push_back(string("FileAlignment: ") + printmemory(&opheader.FileAlignment, 4));
    FileAlignment = opheader.FileAlignment;
    printbuffer.push_back(string("SizeOfImage: ") + printmemory(&opheader.SizeOfImage, 4));
    printbuffer.push_back(string("SizeOfHeaders: ") + printmemory(&opheader.SizeOfHeaders, 4));
    SizeOfHeaders = opheader.SizeOfHeaders;
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
    memcpy(&reloc_directory, &datadirectory[5], sizeof(IMAGE_DATA_DIRECTORY));
    delete[] datadirectory;
}

void parse_nt_header() {
    printbuffer.push_back(string(60, '='));
    DWORD Signature = 0;
    _fseeki64(fp, e_lfanew, SEEK_SET);
    fread(&Signature, 4, 1, fp);
    if(Signature != 0x4550) {
        throw std::runtime_error(string("[!]nt header magic error: ") + printmemory(&Signature, 4));
    }
    printbuffer.push_back("[*]NT header:");
    printbuffer.push_back(string("Signature: ") + printmemory(&Signature, 4));
    parse_file_header();
    parse_optional_header();
}

void parse_section_header() {
    printbuffer.push_back(string(60, '='));
    printbuffer.push_back(string("[*]section header raw: ") + printmemory(&sec_header_raw, 4));
    printbuffer.push_back(string(60, '-'));
    _fseeki64(fp, sec_header_raw, SEEK_SET);
    IMAGE_SECTION_HEADER secheader;
    auto print_section_name = [&]() -> string {
        string res;
        for(int i = 0; i < IMAGE_SIZEOF_SHORT_NAME && secheader.Name[i]; i++) {
            res += secheader.Name[i];
        }
        return res;
    };
    for(WORD i = 0; i < NumberOfSections; i++) {
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
    if(rva < SizeOfHeaders)
        return rva;
    for(size_t i = 0; i < section_header.size(); i++) {
        if(rva >= section_header[i].VirtualAddress
            && (i == section_header.size()-1
                || rva < section_header[i+1].VirtualAddress)) {
                    DWORD offset = rva - section_header[i].VirtualAddress;
                    DWORD raw = section_header[i].PointerToRawData / FileAlignment * FileAlignment + offset;
                    return raw;
                }
    }
    throw std::runtime_error(string("[!]rva to raw wrong. WRONG RVA: ") + printmemory(&rva, sizeof(rva)));
}

void get_func_info(DWORD st_func_rva) {
    DWORD raw = rva_to_raw(st_func_rva);
    _fseeki64(fp, raw, SEEK_SET);
    WORD ordinal;
    fread(&ordinal, sizeof(ordinal), 1, fp);
    printbuffer.push_back(string("rva: ") + printmemory(&st_func_rva, sizeof(st_func_rva))
        + "\traw: " + printmemory(&raw, sizeof(raw))
        + "\tordinal: " + printmemory(&ordinal, sizeof(ordinal)));
    char s[256];
    fread(s, 1, 256, fp);
    s[255] = 0;
    printbuffer.push_back(string("func name: ") + s);
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
            vec_st_func.push_back((DWORD)st_func_rva);
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
    DWORD raw;
    DWORD end_of_section = 0;
    if(iat_rva < SizeOfHeaders) {
        raw = iat_rva;
        end_of_section = SizeOfHeaders;
    } else {
        for(size_t i = 0; i < section_header.size(); i++) {
            if(iat_rva >= section_header[i].VirtualAddress
                && (i == section_header.size()-1
                    || iat_rva < section_header[i+1].VirtualAddress)) {
                        DWORD offset = iat_rva - section_header[i].VirtualAddress;
                        DWORD _raw = section_header[i].PointerToRawData / FileAlignment * FileAlignment + offset;
                        raw = _raw;
                        end_of_section = section_header[i].PointerToRawData + section_header[i].SizeOfRawData;
                        break;
                    }
        }
        if(end_of_section == 0)
            throw std::runtime_error(string("[!]rva to raw wrong. WRONG RVA: ") + printmemory(&iat_rva, sizeof(iat_rva)));
    }
    DWORD size = import_directory.Size;
    printbuffer.push_back(string("[*]IAT\nraw: ") + printmemory(&raw, 4) + "\tsize: " + printmemory(&size, 4));
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
        s[255] = 0;
        printbuffer.push_back(string("DLL name: ") + s); 
        print_rva_raw("OriginalFirstThunk", iid.OriginalFirstThunk, INT_raw);
        print_rva_raw("Name", iid.Name, name_raw);
        print_rva_raw("FirstThunk", iid.FirstThunk, IAT_raw);

        printbuffer.push_back("\n[*]function imported: ");

        if(INT_raw == 0) INT_raw = IAT_raw;
        parse_INT(INT_raw);

        _fseeki64(fp, pos, SEEK_SET);

        if(_ftelli64(fp) >= end_of_section) {
            break;
        }
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

    auto print_rva_raw = [](string name, DWORD rva, DWORD raw) {
        printbuffer.push_back(name + " RVA: " + printmemory(&rva, 4)
            + "\tRAW: " + printmemory(&raw, 4));
    };

    DWORD ied_name_raw = rva_to_raw(ied.Name);
    _fseeki64(fp, ied_name_raw, SEEK_SET);
    char s[256];
    fread(s, 1, 256, fp);
    s[255] = 0;
    printbuffer.push_back(string("DLL name: ") + s); 
    printbuffer.push_back(string(60, '-'));
    printbuffer.push_back(string("NumberOfFunctions: ") + printmemory(&ied.NumberOfFunctions, 4));
    printbuffer.push_back(string("NumberOfNames: ") + printmemory(&ied.NumberOfNames, 4));

    DWORD AddressOfFunctions_raw = rva_to_raw(ied.AddressOfFunctions);
    DWORD AddressOfNames_raw = rva_to_raw(ied.AddressOfNames); 
    DWORD AddressOfNameOrdinals_raw = rva_to_raw(ied.AddressOfNameOrdinals); 

    print_rva_raw("AddressOfFunctions", ied.AddressOfFunctions, AddressOfFunctions_raw);
    print_rva_raw("AddressOfNames", ied.AddressOfNames, AddressOfNames_raw);
    print_rva_raw("AddressOfNameOrdinals", ied.AddressOfNameOrdinals, AddressOfNameOrdinals_raw);
    printbuffer.push_back(string(60, '-'));
    printbuffer.push_back("[*]named function: ");
    bool *named_ordinal = new bool[ied.NumberOfFunctions];
    for(DWORD i = 0; i < ied.NumberOfFunctions; i++) {
        named_ordinal[i] = false;
    }
    vector<WORD> vec_ordinal;
    vector<DWORD> vec_name_rva;
    vector<DWORD> vec_addr_rva;
    _fseeki64(fp, AddressOfNameOrdinals_raw, SEEK_SET);
    for(DWORD i = 0; i < ied.NumberOfNames; i++) {
        WORD ordinal;
        fread(&ordinal, sizeof(WORD), 1, fp);
        vec_ordinal.push_back(ordinal);
        named_ordinal[ordinal] = true;
    }
    _fseeki64(fp, AddressOfNames_raw, SEEK_SET);
    for(DWORD i = 0; i < ied.NumberOfNames; i++) {
        DWORD name_rva;
        fread(&name_rva, sizeof(DWORD), 1, fp);
        vec_name_rva.push_back(name_rva);
    }
    _fseeki64(fp, AddressOfFunctions_raw, SEEK_SET);
    for(DWORD i = 0; i < ied.NumberOfFunctions; i++) {
        DWORD addr_rva;
        fread(&addr_rva, sizeof(DWORD), 1, fp);
        vec_addr_rva.push_back(addr_rva);
    }
    for(DWORD i = 0; i < ied.NumberOfNames; i++) {
        WORD ordinal = vec_ordinal[i];
        DWORD name_raw = rva_to_raw(vec_name_rva[i]);
        char s[256];
        _fseeki64(fp, name_raw, SEEK_SET);
        fread(s, 1, 256, fp);
        s[255] = 0;
        string name = s;
        DWORD addr_raw = rva_to_raw(vec_addr_rva[ordinal]);
        printbuffer.push_back(string("Ord: ") + printmemory(&ordinal, sizeof(ordinal))
            + "\t" + name);
        printbuffer.push_back(string("RVA: ") + printmemory(&vec_addr_rva[i], sizeof(DWORD))
            + "\tRAW: " + printmemory(&addr_raw, sizeof(DWORD)));
    }
    printbuffer.push_back(string(60, '-'));
    if(ied.NumberOfFunctions == ied.NumberOfNames) {
        printbuffer.push_back("[*]no unamed function");
        delete[] named_ordinal;
        return;
    }
    printbuffer.push_back("[*]unamed function: ");
    for(DWORD i = 0; i < ied.NumberOfFunctions; i++) {
        if(named_ordinal[i] == true)
            continue;
        DWORD addr_raw = rva_to_raw(vec_addr_rva[i]);
        printbuffer.push_back(string("Ord: ") + printmemory(&i, sizeof(WORD))
            + "\tRVA" + printmemory(&vec_addr_rva[i], sizeof(DWORD))
            + "\tRAW" + printmemory(&addr_raw, sizeof(DWORD)));
    }
    delete[] named_ordinal;
}

void parse_rt() {
    printbuffer.push_back(string(60, '='));
    DWORD rt_rva = reloc_directory.VirtualAddress;
    DWORD raw = rva_to_raw(rt_rva);
    if(raw == 0) {
        printbuffer.push_back("[*]Don't have relocation table");
        return;
    }
    DWORD size = reloc_directory.Size;
    printbuffer.push_back(string("[*]Relocation Table\nraw: ") + printmemory(&raw, 4) + "\tsize: " + printmemory(&size, 4));
    // IMAGE_BASE_RELOCATION ibr; 
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    try {
        if(argc < 2) {
            throw std::runtime_error("[!]Missing parameter: file_path");
        }
        fp = clre::FileMng(argv[1], "rb");
        parse_dos_header();
        parse_nt_header();
        parse_section_header();
        parse_iat();
        parse_eat();
        parse_rt();
        output();
    } catch (const std::exception &e) {
        output();
        std::cout << e.what() << "\n";
    }
    std::cout << std::flush;
    system("pause");
    return 0;
}
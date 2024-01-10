//#define _CRT_SECURE_NO_WARNINGS
//g++ .\crypto.cpp -o crypto -Wall -Wextra -lshlwapi -O3

#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <time.h>
#include "myaes.h"
#include "TraverseFiles.h"

constexpr auto READSIZE = 1021 * 1024 * 1024;
constexpr auto KEYSIZE = 32;
constexpr auto IVSIZE = 16;

static uint8 key[KEYSIZE];
static uint8 iv[IVSIZE];

static uint8 buff[READSIZE];
static uint8 alignbuff[IVSIZE];

static bool if_encode = 0;

static uint8 filehead[] = { 0xE8, 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3, 0xC0 };

static std::vector<std::string> FileOpErr;
static clock_t start, end;

static void geniv(void) {
    srand((unsigned int)time(nullptr));
    auto geniv = [](const int i) { ((short*)&iv)[i] = (short)rand(); };
    geniv(0); geniv(1); geniv(2); geniv(3);
    geniv(4); geniv(5); geniv(6); geniv(7);
}

static void EncodeFile(FILE *fp, const std::string &filepath) {
    fread(buff, 1, 8, fp);
    if ((*(size_t*)buff & 0xFFFFFFFFFFFFFF) == 0xC390909090E9E8) {
        fclose(fp);
        FileOpErr.push_back(std::string("[!]file has been encode : ") + filepath);
        throw FileOpErr.back();
    }
    
    std::string filepathbackup = filepath + ".tmp";
    FILE* tmp = fopen(filepathbackup.c_str(), "wb");
    if(!tmp) {
        fclose(fp);
        FileOpErr.push_back(std::string("[!]open file : ") + filepathbackup + " fail");
        throw FileOpErr.back();
    }

    size_t szRead = 0, szFile = 0;
    uint8 align = 0;
    geniv();
    _fseeki64(fp, 0, SEEK_END);
    szFile = _ftelli64(fp);
    align = szFile % 16;
    filehead[7] &= 0xF0;
    if (align) {
        fwrite(alignbuff, 1, 16 - align, fp);
        filehead[7] ^= align;
    }
    _fseeki64(fp, 0, SEEK_SET);
    fwrite(filehead, 1, 8, tmp);
    fwrite(iv, 1, IVSIZE, tmp);
    struct AES_ctx aes;
    AES_init_ctx(&aes, key, iv);
    while((szRead = fread(buff, 1, READSIZE, fp))) {
        AES_CBC_encrypt_buffer(&aes, buff, szRead);
        fwrite(buff, 1, szRead, tmp);
    }
    fclose(tmp);
    fclose(fp);
    remove(filepath.c_str());
    rename(filepathbackup.c_str(), filepath.c_str());
}

static void DecodeFile(FILE *fp, const std::string &filepath) {
    fread(buff, 1, 8, fp);
    if ((*(size_t*)buff & 0xFFFFFFFFFFFFFF) != 0xC390909090E9E8) {
        fclose(fp);
        FileOpErr.push_back(std::string("[!]file hasn't been encode : ") + filepath);
        throw FileOpErr.back();
    }

    std::string filepathbackup = filepath + ".tmp";
    FILE* tmp = fopen(filepathbackup.c_str(), "wb");
    if(!tmp) {
        fclose(fp);
        FileOpErr.push_back(std::string("[!]open file : ") + filepathbackup + " fail");
        throw FileOpErr.back();
    }

    size_t chunks = 0, szRead = 0;
    uint8 align = 0;
    _fseeki64(fp, 0, SEEK_END);
    chunks = (_ftelli64(fp) - 24) / READSIZE;
    if((_ftelli64(fp) - 24) % READSIZE)
        chunks++;
    _fseeki64(fp, 8, SEEK_SET);
    align = *(buff + 7) & 0xF;
    struct AES_ctx aes;
    fread(buff, 1, IVSIZE, fp);
    memcpy(iv, buff, IVSIZE);
    AES_init_ctx(&aes, key, iv);
    while((szRead = fread(buff, 1, READSIZE, fp))) {
        chunks--;
        AES_CBC_decrypt_buffer(&aes, buff, szRead);
        if(chunks) {
            fwrite(buff, 1, szRead, tmp);
            continue;
        }
        if(align) {
            fwrite(buff, 1, szRead - 16 + align, tmp);
            break;
        } else {
            fwrite(buff, 1, szRead, tmp);
            break;
        }
    }
    fclose(tmp);
    fclose(fp);
    remove(filepath.c_str());
    rename(filepathbackup.c_str(), filepath.c_str());
}

static void EncodeAndDecodeFile(const char *dirpath)
{
    std::string filepath = dirpath;
    FILE* fp = fopen(filepath.c_str(), "rb+");
    if(!fp) {
        FileOpErr.push_back(std::string("[!]open file : ") + filepath + " fall");
        throw FileOpErr.back();
    }
    if (if_encode) {
        EncodeFile(fp, filepath);
    } else {
        DecodeFile(fp, filepath);
    }
}

static void GenKey() {
    std::string s;
    std::cin >> s;
    auto len = s.size();
    for(int i = 0; i < KEYSIZE; i++) {
        key[i] = s[i % len];
    }
}

static void CmdUI(int argc, char *argv[]) {
    if(argc < 2) {
        throw "[!]Missing parameter: file_path";
    }
    std::cout << "[*]BEGIN\n" "[FILE_PATH]" << argv[1] << "\n";
    std::cout << std::flush;
    std::cin >> if_encode;
    GenKey();
    start = clock();
    TraversalFiles(argv[1], EncodeAndDecodeFile);
}

static void PrintFinalInfo() {
    if(!FileOpErr.empty())
        for(const auto &i : FileOpErr)
            std::cout << i << "\n";
    end = clock();
    std::cout << "[+]total time is " << (double)(end - start) / CLOCKS_PER_SEC << "s\n";
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout << std::fixed << std::setprecision(3);

    try {
        CmdUI(argc, argv);
        PrintFinalInfo();
    } catch(const char *e) {
        std::cout << e << "\n";
    } catch(const std::string &e) {
        std::cout << e << "\n";
    }

    std::cout << std::flush;
    system("pause");
    return 0;
}
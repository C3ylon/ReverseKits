#include <iostream>
#include <string>
#include <vector>
#include "filemng.h"

using std::string;
using std::vector;

bool is_x64;
vector<string> printbuffer;

void is_pe_plus() {

}

void parse_dos_header() {

}

int main() {
    std::ios_base::sync_with_stdio(false);
    string filepath;
    std::cin >> filepath;
    clre::FileMng fp(filepath.c_str(), "rb");
    is_pe_plus();
    parse_dos_header();
    return 0;
}
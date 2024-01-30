#ifndef HEADER__H_
#define HEADER__H_

#define WIN32_LEAN_AND_MEAN
// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tlhelp32.h>

#undef Module32First
#undef Module32Next
#undef MODULEENTRY32
#undef PMODULEENTRY32
#undef LPMODULEENTRY32

#include <iostream>
#include <string>
#include <stdexcept>
using std::cin;
using std::cout;
using std::endl;
using std::string;

#endif
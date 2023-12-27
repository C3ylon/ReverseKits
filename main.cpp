#include "handle.h"
#include <tlhelp32.h>
#include <iostream>
#include "filemng.h"
using std::cout;
using std::endl;




int main() {
    clre::FileMng fp("./test.txt", "rb");
    _fseeki64(fp, 1, 0);
    // fread(0, 1,1,fp);
    char ch = 0;
    fread(&ch, 1,1,fp);
    cout << ch<<endl;
    return 0;
}
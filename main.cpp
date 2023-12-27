#include "handle.h"
#include <tlhelp32.h>
#include <iostream>
#include "filemng.h"
using std::cout;
using std::endl;




int main() {
    clre::FileMng fp("./test.txt", "wb");
    fwrite("avbde", 1, 5, fp);
    return 0;
}
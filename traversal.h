#ifndef TRAVERSAL__H_
#define TRAVERSAL__H_

#include <stdio.h>
#include <sys/stat.h>
#include <io.h>
#include <string.h>

#ifndef MAX_PATH
    #define MAX_PATH 260
#endif


void TraversalFiles(const char *dir)
{
    struct _stat64 sbuff;
    if(_stat64(dir, &sbuff) == -1) {
        printf("[!]find file or folder path fail\n");
        return;
    } 
    if(sbuff.st_mode & _S_IFREG) {
        printf("[*]file size: %I64d bytes\n", sbuff.st_size);
    }
    else if(sbuff.st_mode & S_IFDIR) {
        char dirNew[MAX_PATH];
        strcpy(dirNew, dir);
        strcat(dirNew, "\\*.*");
        struct __finddata64_t findData;
        intptr_t handle = _findfirst64(dirNew, &findData);
        if (handle == -1) {
            printf("[!]find path fail\n");
            return;
        }
        do {
            if(findData.attrib & _A_SUBDIR) {
                if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0) {
                    continue;
                }
                printf("[FOLDER]%s\n", findData.name);
                memset(dirNew, 0, MAX_PATH);
                strcpy(dirNew, dir);
                strcat(dirNew, "\\");
                strcat(dirNew, findData.name);
                TraversalFiles(dirNew);
            }
            else {
                printf("[FILE]%s\t%I64d bytes\t", findData.name, findData.size);
            }
        } while(_findnext64(handle, &findData) == 0);
        _findclose(handle);
    }
}


#endif
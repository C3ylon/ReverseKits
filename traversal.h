#ifndef TRAVERSAL__H_
#define TRAVERSAL__H_

#include <stdio.h>
#include <sys/stat.h>
#include <io.h>
#include <string.h>

#ifdef LOG__H_
    #include "./log.h"
    #define LOG_INFO(...) log_info(__VA_ARGS__)
    #define LOG_ERR(...) log_error(__VA_ARGS__)
#else
    #define LOG_INFO(...)
#endif

#ifndef MAX_PATH
    #define MAX_PATH 260
#endif


void TraversalFiles(const char *dir)
{
    struct _stat64 sbuff;
    if(_stat64(dir, &sbuff) == -1) {
        LOG_ERR("[!]find file or folder path fail");
        return;
    }
    if(sbuff.st_mode & _S_IFREG) {
        LOG_INFO("[*]file size: %I64d bytes", sbuff.st_size);
    } else if(sbuff.st_mode & S_IFDIR) {
        char dirNew[MAX_PATH];
        strcpy(dirNew, dir);
        strcat(dirNew, "\\*.*");
        struct __finddata64_t findData;
        intptr_t handle = _findfirst64(dirNew, &findData);
        if (handle == -1) {
            LOG_INFO("[!]find path fail");
            return;
        }
        do {
            if(findData.attrib & _A_SUBDIR) {
                if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0) {
                    continue;
                }
                LOG_INFO("[FOLDER]%s", findData.name);
                memset(dirNew, 0, MAX_PATH);
                strcpy(dirNew, dir);
                strcat(dirNew, "\\");
                strcat(dirNew, findData.name);
                TraversalFiles(dirNew);
            }
            else {
                LOG_INFO("[FILE]%s\t%I64d bytes", findData.name, findData.size);
            }
        } while(_findnext64(handle, &findData) == 0);
        _findclose(handle);
    }
}


#endif
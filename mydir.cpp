//
// Created by sun on 11/13/16.
//

#include "mydir.h"

#if defined (__unix__) || defined (__APPLE__)
#include <sys/types.h>
#include <dirent.h>
mylist<mystring> mydir(const mystring &directory) {
    mylist<mystring> dirs;
    mystring dir(directory);
    if (dir.back() != '\0')
        dir.push_back('\0');
    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir.begin());
    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type == DT_REG)
                dirs.push_back(ep->d_name);
        }
        closedir(dp);
    }
    return dirs;
}
#elif defined(_WIN32)
#include <windows.h>
// To be tested on windows
mylist<mystring> mydir(const mystring &directory) {
    mylist<mystring> dirs;
    mystring dir(directory);

    WIN32_FIND_DATA findfiledata;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char fullpath[MAX_PATH];
    GetFullPathNameA(path.c_str(), MAX_PATH, fullpath, 0);
    std::string fp(fullpath);

    if (dir.back() == '\0')
        dir.pop_back();
    if (dir.back() == '\\' || dir.back() == '/')
        dir.pop_back();
    for (std::size_t i = 0; i < dir.size(); ++i)
        if (dir[i] == '/')
            dir[i] = '\\';
    dir += "\\*"
    dir.push_back('\0');

    hFind = FindFirstFileA((LPCSTR) dir.begin(), &findfiledata);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findfiledata.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
                dirs.push_back(findfiledata.cFileName);
        }
        while (FindNextFileA(hFind, &findfiledata));
    }
}
#endif
//
// Created by sun on 11/13/16.
//

#include "mydir.h"

#if defined (__unix__) || defined (__APPLE__)
#include <sys/types.h>
#include <dirent.h>
myvector<mystring> mydir(const mystring &directory) {
    myvector<mystring> dirs;
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
myvector<mystring> mydir(const mystring &directory) {
    myvector<mystring> dirs;
    mystring dir(directory);

    WIN32_FIND_DATAA findfiledata;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    if (dir.back() == '\\' || dir.back() == '/')
        dir.pop_back();
    for (std::size_t i = 0; i < dir.size(); ++i)
        if (dir[i] == '/')
            dir[i] = '\\';
	dir += "\\*";

    hFind = FindFirstFileA((LPCSTR) dir.c_str(), &findfiledata);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findfiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                dirs.push_back(findfiledata.cFileName);
        }
        while (FindNextFileA(hFind, &findfiledata));
    }
    return dirs;
}
#else
#error "Unknown system."
#endif

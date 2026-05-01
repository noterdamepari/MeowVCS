#ifndef MEOW_H
#define MEOW_H

    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include "sha1.h"
    #include "zlib.h"

    #ifdef _WIN32
        #include <direct.h>
        #define mkdir(path, mode) _mkdir(path)
        #define getcwd(buf, size) _getcwd(buf, size)
    #else
        #include <sys/stat.h>
        #include <sys/types.h>
        #include <unistd.h>
    #endif

#endif

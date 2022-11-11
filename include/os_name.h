#ifndef __OS_NAME_H__
#define __OS_NAME_H__

//Provides handy macros to get used OS

#if defined(__gnu_linux__)
    #define OS_LINUX 1 //Sorry richard ...$
    #define OS_NAME "GNU/Linux"
#elif defined(_WIN32)
    #define OS_WINDOWS 1
    #define OS_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
    #define OS_APPLE 1
    #define OS_NAME "Apple OSX"
#endif

#if !defined(OS_LINUX)
    #define OS_LINUX 0
#elif !defined(OS_WINDOWS)
    #define OS_WINDOWS 0
#elif !defined(OS_APPLE)
    #define OS_APPLE 0
#endif


#endif //__OS_NAME_H__
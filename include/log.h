#ifndef __DEBUG_H__
#define __DEBUG_H__

/*
    This is an header only, simplistic "logging" system.
    It must be include once with #define LOG_IMPLEMENTATION
    And then can be included normaly throuhout the program
*/

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<stdarg.h>
#include<inttypes.h>

#define ANSI_RES "\033[0m"

extern void __log_msg(int log_level, const char* filename, const int line_number, const char* function_name,char* message, ...);
extern void *__xmalloc(unsigned long bytes, const char* filename, const int line, const char* function_name);


//Stringify
#define STRFY(s) #s

//Logs an information
#define LOG(s) __log_msg(0, __FILE__, __LINE__, __func__, s);
#define LOGF(s, ...) __log_msg(0, __FILE__, __LINE__, __func__, s, __VA_ARGS__);

//Logs a success
#define SUC(s) __log_msg(1, __FILE__, __LINE__, __func__, s);
#define SUCF(s, ...) __log_msg(1, __FILE__, __LINE__, __func__, s, __VA_ARGS__);

//Logs a warning
#define WARN(s) __log_msg(2, __FILE__, __LINE__, __func__, s);
#define WARNF(s, ...) __log_msg(2, __FILE__, __LINE__, __func__, s, __VA_ARGS__);

//Logs a non-fatal error
#define ERR(s) __log_msg(3, __FILE__, __LINE__, __func__, s);
#define ERRF(s, ...) __log_msg(3, __FILE__, __LINE__, __func__, s, __VA_ARGS__);

//Logs a fatal error (Exits program)
#define FAIL(s) __log_msg(4, __FILE__, __LINE__, __func__, s); abort();
#define FAILF(s, ...) __log_msg(4, __FILE__, __LINE__, __func__, s, __VA_ARGS__); abort();

//Prints an int with its name
#define VARI32(v) LOGF("%s: %"PRIi32, #v, v);

//Prints an char (hex) with its name
#define VARI8X(v) LOGF("%s: 0x%x", #v, v);

//Prints an uint with its name
#define VARU32(v) LOGF("%s: %"PRIu32, #v, v);

//Prints a long with its name
#define VARI64(v) LOGF("%s: %"PRIi64, #v, v);

//Prints a ulong with its name
#define VARU64(v) LOGF("%s: %"PRIu64, #v, v);

//Prints a string with its name
#define VARSTR(v) LOGF("%s: %s", #v, v);

//Assert fail (crash)
#define ASSERT(s, m, ...) if(s) {} else {ERRF("Assertion failed ( %s ) :", #s); __log_msg(4, __FILE__, __LINE__, __func__, m, __VA_ARGS__); abort();}

//Assert fail (prints success)
#define ASSERTS(s, m) if(s) {__log_msg(1, __FILE__, __LINE__, __func__, "Assertion passes %s", #s);} else {ERRF("Assertion failed ( %s ) :", #s); __log_msg(4, __FILE__, __LINE__,__func__, m); abort();}

//Safe malloc
#define xmalloc(l) __xmalloc(l, __FILE__, __LINE__, __func__)

#endif

#ifdef LOG_IMPLEMENTATION
//Prefixes
static char* __log_prefixes[] =
{
    "\033[34mINFO" ANSI_RES,
    "\033[1;32mSUCCESS" ANSI_RES,
    "\033[33mWARN" ANSI_RES,
    "\033[31mERROR" ANSI_RES,
    "\033[1;31mFAIL" ANSI_RES
};

#ifdef LOG_TO_FILE
static char* __log_prefixes_no_color[] =
{
    "INFO",
    "SUCCESS",
    "WARN",
    "ERROR",
    "FAIL"
};
#endif

#ifdef LOG_TO_FILE
static FILE* log__logf_;
static int log__logtf_ = 0;

#define START_LOG_FILE(p) log__logf_ = fopen(p, "w"); log__logtf_ = 42; LOG("Begining log file");
#define END_LOG_FILE()  LOG("Log file end"); fclose(log__logf_); log__logf_ = NULL;
#endif

void __log_msg(int log_level, const char* filename, const int line_number, const char* function_name,char* message, ...)
{

#ifdef LOG_FUNC
    printf("[%s:%d in %s][%s] ", filename, line_number, function_name,__log_prefixes[log_level]);
#else
    printf("[%s:%d][%s] ", filename, line_number, __log_prefixes[log_level]);
#endif


    va_list argptr;
    va_start(argptr, message);
    vfprintf(stdout, message, argptr);
    va_end(argptr);

    printf("\n");

#ifdef LOG_TO_FILE
    if(log__logtf_)
    {

        #ifdef LOG_FUNC
            fprintf(log__logf_, "[%s:%d in %s][%s] ", filename, line_number, function_name, __log_prefixes_no_color[log_level]);
        #else
            fprintf(log__logf_, "[%s:%d][%s] ", filename, line_number, __log_prefixes_no_color[log_level]);
        #endif
        va_list argptrf;
        va_start(argptrf, message);
        vfprintf(log__logf_, message, argptrf);
        va_end(argptrf);
        fprintf(log__logf_, "\n");
        fflush(log__logf_);
    }
    if(!log__logtf_)
    {   
        FAIL("Log file was not opened");
    }
#endif
}

/// Mallocs that checks pointer integrity
void *__xmalloc(unsigned long bytes, const char* filename, const int line, const char* function_name)
{
    void* ptr = malloc(bytes);
    if(!ptr)
    {
        __log_msg(4, filename, line, function_name, "Memory allocation failed ( %lu bytes ) !", bytes);
        abort();
    }
    return ptr;
}
#endif // LOG_IMPLEMENTATION


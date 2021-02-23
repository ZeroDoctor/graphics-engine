#pragma once

#include "build_order.hpp"

// TODO: maybe create a namespace here
inline void formatter(const char* prepend, const char* message, va_list args) 
{
    vprintf((std::string(prepend) + message).c_str(), args);
}

inline void printfw(const char* msg, ...) 
{
    va_list args;
    va_start(args, msg);
    formatter("\033[1;33mWARN\033[0m: ", msg, args);
    va_end(args);
}

inline void printfe(const char* msg, ...) 
{ 
    va_list args;
    va_start(args, msg);
    formatter("\033[1;31mERRO\033[0m: ", msg, args);
    va_end(args);
}

inline void printfi(const char* msg, ...) 
{ 
    va_list args;
    va_start(args, msg);
    formatter("\033[1;36mINFO\033[0m: ", msg, args);
    va_end(args);
}

inline void printfv(const char* msg, ...) 
{ 
    va_list args;
    va_start(args, msg);
    formatter("\033[1;32mVBSE\033[0m: ", msg, args);
    va_end(args);
}

// may not work sometimes on vscode terminal
inline void printff(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    formatter("\033[1;31mFATL\033[0m:", msg, args);
    va_end(args);

    throw std::runtime_error("throwing error");
}

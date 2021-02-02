#pragma once

#include <fstream>
#include "printer.hpp"

inline void printff(const char*, ...);

static std::vector<char> read_shader(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary | std::ios::in);

    if(!file.is_open()) {
        printff("Failed to open shader file\n");
    }

    
    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), file_size);

    if(file_size <= 0) {
        printff("Failed to read shader correctly\n");
    }

    file.close();

    return buffer;
}
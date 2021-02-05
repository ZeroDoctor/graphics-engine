#pragma once

#include "build_order.hpp"

class Surface 
{
public:
    Surface(uint32_t, uint32_t);
    ~Surface();


private:
    uint32_t m_width;
    uint32_t m_height;
    GLFWwindow* m_window;
};

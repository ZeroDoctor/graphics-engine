#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <array>
#include <memory>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "printer.hpp"
#include "util.hpp"
#include "validator.hpp"
#include "initializer.hpp"

// Note: many enums have a max element, however this only works for pipeline enums
#define ZERO_BIT VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM




#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vma/vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <foundation/platform.hpp>

#define VK_CHECK(x)                                                           \
    do                                                                        \
    {                                                                         \
        VkResult err = x;                                                     \
        if (err)                                                              \
        {                                                                     \
            spdlog::error("Detected Vulkan error: {}", string_VkResult(err)); \
            abort();                                                          \
        }                                                                     \
    } while (0)

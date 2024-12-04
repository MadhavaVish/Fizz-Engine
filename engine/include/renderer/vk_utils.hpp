#pragma once
#include <renderer/vk_types.hpp>

namespace vkutil
{

    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
}
#pragma once
#include <renderer/vk_types.hpp>

namespace fizzengine
{
    struct Texture
    {
        VkImage m_image;
        VkImageView m_image_view;
        VkFormat m_format;
        VkImageLayout m_image_layout;
        VmaAllocation m_vma_allocation;

        u16 width = 1;
        u16 height = 1;
        u16 depth = 1;
        u8 mipmaps = 1;
        u8 flags = 0;
    };
}
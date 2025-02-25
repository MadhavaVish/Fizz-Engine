#pragma once
#include <renderer/vk_types.hpp>

namespace fizzengine {

struct Texture {
    VkImage       m_image;
    VkImageView   m_image_view;
    VkSampler     m_sampler;
    VkFormat      m_format;
    VkImageLayout m_image_layout;
    VmaAllocation m_vma_allocation;

    u16           width   = 1;
    u16           height  = 1;
    u16           depth   = 1;
    u8            mipmaps = 1;
    u8            flags   = 0;
};

struct DescriptorLayoutBuilder {

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void                                      add_binding(uint32_t binding, VkDescriptorType type);
    void                                      clear();
    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages,
                                void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {

    struct PoolSizeRatio {
        VkDescriptorType type;
        float            ratio;
    };

    VkDescriptorPool pool;

    void init_pool(VkDevice device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios);
    void clear_descriptors(VkDevice device);
    void destroy_pool(VkDevice device);

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

} // namespace fizzengine
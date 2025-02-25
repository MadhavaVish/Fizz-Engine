#pragma once
#include <renderer/vk_types.hpp>
#include <slang/slang.h>

namespace vkutil {
void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout,
                      VkImageLayout new_layout);

void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination,
                         VkExtent2D srcSize, VkExtent2D dstSize);

slang::IGlobalSession*  CreateSlangSession();

slang::ICompileRequest* CreateCompileRequest(slang::IGlobalSession* session);

VkShaderModule CompileSlangShader(VkDevice device, const char* shaderPath, const char* entryPoint,
                                  VkShaderStageFlagBits stage);
} // namespace vkutil
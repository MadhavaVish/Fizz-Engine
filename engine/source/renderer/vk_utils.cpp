#include <renderer/vk_initializers.hpp>
#include <renderer/vk_utils.hpp>


namespace vkutil {
void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout,
                      VkImageLayout new_layout) {
    VkImageMemoryBarrier2 image_barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    image_barrier.pNext            = nullptr;

    image_barrier.srcStageMask     = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.srcAccessMask    = VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.dstStageMask     = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.dstAccessMask    = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    image_barrier.oldLayout        = current_layout;
    image_barrier.newLayout        = new_layout;

    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                                         ? VK_IMAGE_ASPECT_DEPTH_BIT
                                         : VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange = vkinit::image_subresource_range(aspect_mask);
    image_barrier.image            = image;

    VkDependencyInfo dep_info{};
    dep_info.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep_info.pNext                   = nullptr;

    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers    = &image_barrier;

    vkCmdPipelineBarrier2(cmd, &dep_info);
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination,
                         VkExtent2D srcSize, VkExtent2D dstSize) {
    VkImageBlit2 blit_region{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};

    blit_region.srcOffsets[1].x               = srcSize.width;
    blit_region.srcOffsets[1].y               = srcSize.height;
    blit_region.srcOffsets[1].z               = 1;

    blit_region.dstOffsets[1].x               = dstSize.width;
    blit_region.dstOffsets[1].y               = dstSize.height;
    blit_region.dstOffsets[1].z               = 1;

    blit_region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount     = 1;
    blit_region.srcSubresource.mipLevel       = 0;

    blit_region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount     = 1;
    blit_region.dstSubresource.mipLevel       = 0;

    VkBlitImageInfo2 blit_info{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
    blit_info.dstImage       = destination;
    blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blit_info.srcImage       = source;
    blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blit_info.filter         = VK_FILTER_LINEAR;
    blit_info.regionCount    = 1;
    blit_info.pRegions       = &blit_region;

    vkCmdBlitImage2(cmd, &blit_info);
}

slang::IGlobalSession* CreateSlangSession() {
    slang::IGlobalSession* session = nullptr;
    slang::createGlobalSession(&session);
    return session;
}

slang::ICompileRequest* CreateCompileRequest(slang::IGlobalSession* session) {
    slang::ICompileRequest* request = nullptr;
    session->createCompileRequest(&request);
    return request;
}

VkShaderModule CompileSlangShader(VkDevice device, const char* shaderPath, const char* entryPoint,
                                  VkShaderStageFlagBits stage) {
    // Create Slang session
    slang::IGlobalSession*  slangSession   = CreateSlangSession();
    slang::ICompileRequest* compileRequest = CreateCompileRequest(slangSession);

    // Setup compilation parameters
    int                     targetIndex = compileRequest->addCodeGenTarget(SLANG_SPIRV);
    compileRequest->setTargetProfile(targetIndex, slangSession->findProfile("spirv_1_5"));

    // Add translation unit (your shader file)
    int translationUnitIndex =
        compileRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    compileRequest->addTranslationUnitSourceFile(translationUnitIndex, shaderPath);

    // Map Vulkan stage to Slang stage
    SlangStage slangStage = SLANG_STAGE_NONE;
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
        slangStage = SLANG_STAGE_VERTEX;
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        slangStage = SLANG_STAGE_FRAGMENT;
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        slangStage = SLANG_STAGE_COMPUTE;
        break;
    // Add other stages as needed
    default:
        throw std::runtime_error("Unsupported shader stage");
    }

    // Add entry point
    int entryPointIndex =
        compileRequest->addEntryPoint(translationUnitIndex, entryPoint, slangStage);

    // Compile
    const SlangResult compileResult = compileRequest->compile();

    // Check for errors
    if (SLANG_FAILED(compileResult)) {
        const char* diagnostics = compileRequest->getDiagnosticOutput();
        printf("Shader compilation failed:\n%s\n", diagnostics);
        return VK_NULL_HANDLE;
    }

    // Get compiled SPIR-V code
    compileRequest->getEntryPointCode(entryPointIndex, 0);

    size_t                   spirvSize = 0;
    const void*              spirvData = compileRequest->getCompileRequestCode(&spirvSize);

    // Create Vulkan shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvSize;
    createInfo.pCode    = static_cast<const uint32_t*>(spirvData);

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    // Cleanup Slang resources
    compileRequest->release();

    return shaderModule;
}
} // namespace vkutil
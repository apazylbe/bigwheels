// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ppx/grfx/vk/vk_xr_swapchain.h"
#include "ppx/grfx/vk/vk_device.h"
#include "ppx/grfx/vk/vk_gpu.h"
#include "ppx/grfx/vk/vk_instance.h"
#include "ppx/grfx/vk/vk_queue.h"
#include "ppx/grfx/vk/vk_sync.h"

#include "ppx/grfx/vk/vk_profiler_fn_wrapper.h"

namespace ppx {
namespace grfx {
namespace vk {

Result XRSwapchain::CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo)
{
    std::vector<VkImage> colorImages;
    std::vector<VkImage> depthImages;

    XrSwapchainCreateInfo info = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
    info.arraySize             = 1;
    info.mipCount              = 1;
    info.faceCount             = 1;
    info.format                = ToVkFormat(pCreateInfo->colorFormat);
    info.width                 = pCreateInfo->width;
    info.height                = pCreateInfo->height;
    info.sampleCount           = pCreateInfo->sampleCount;
    info.usageFlags            = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
    CHECK_XR_CALL(xrCreateSwapchain(pCreateInfo->xrSession, &info, &mXrColorSwapchain));

    // Find out how many textures were generated for the swapchain
    uint32_t imageCount = 0;
    CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrColorSwapchain, 0, &imageCount, nullptr));

    std::vector<XrSwapchainImageVulkanKHR> xrImages;
    xrImages.resize(imageCount, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
    CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrColorSwapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)xrImages.data()));
    for (uint32_t i = 0; i < imageCount; i++) {
        colorImages.push_back(xrImages[i].image);
    }

    if (pCreateInfo->depthFormat != grfx::FORMAT_UNDEFINED) {
        info.format     = ToVkFormat(pCreateInfo->depthFormat);
        info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        CHECK_XR_CALL(xrCreateSwapchain(pCreateInfo->xrSession, &info, &mXrDepthSwapchain));

        imageCount = 0;
        CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrDepthSwapchain, 0, &imageCount, nullptr));
        std::vector<XrSwapchainImageVulkanKHR> swapchainDepthImages;
        swapchainDepthImages.resize(imageCount, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
        CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrDepthSwapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)swapchainDepthImages.data()));
        for (uint32_t i = 0; i < imageCount; i++) {
            depthImages.push_back(swapchainDepthImages[i].image);
        }

        PPX_ASSERT_MSG(depthImages.size() == colorImages.size(), "XR depth and color swapchains have different number of images");
    }

    // Transition images from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
    {
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk::Queue*    pQueue    = ToApi(pCreateInfo->pQueue);
        for (uint32_t i = 0; i < colorImages.size(); ++i) {
            VkResult vkres = pQueue->TransitionImageLayout(
                colorImages[i],                     // image
                VK_IMAGE_ASPECT_COLOR_BIT,          // aspectMask
                0,                                  // baseMipLevel
                1,                                  // levelCount
                0,                                  // baseArrayLayer
                1,                                  // layerCount
                VK_IMAGE_LAYOUT_UNDEFINED,          // oldLayout
                newLayout,                          // newLayout
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT); // newPipelineStage
            if (vkres != VK_SUCCESS) {
                PPX_ASSERT_MSG(false, "vk::Queue::TransitionImageLayout failed: " << ToString(vkres));
                return ppx::ERROR_API_FAILURE;
            }
        }
    }

    // Create images.
    {
        for (uint32_t i = 0; i < colorImages.size(); ++i) {
            grfx::ImageCreateInfo imageCreateInfo = {};
            imageCreateInfo.type                  = grfx::IMAGE_TYPE_2D;
            imageCreateInfo.width                 = pCreateInfo->width;
            imageCreateInfo.height                = pCreateInfo->height;
            imageCreateInfo.depth                 = 1;
            imageCreateInfo.format                = pCreateInfo->colorFormat;
            // TODO: use sample count from mCreateInfo.
            imageCreateInfo.sampleCount                     = grfx::SAMPLE_COUNT_1;
            imageCreateInfo.mipLevelCount                   = 1;
            imageCreateInfo.arrayLayerCount                 = 1;
            imageCreateInfo.usageFlags.bits.transferSrc     = true;
            imageCreateInfo.usageFlags.bits.transferDst     = true;
            imageCreateInfo.usageFlags.bits.sampled         = true;
            imageCreateInfo.usageFlags.bits.storage         = true;
            imageCreateInfo.usageFlags.bits.colorAttachment = true;
            imageCreateInfo.pApiObject                      = (void*)(colorImages[i]);

            grfx::ImagePtr image;
            Result         ppxres = GetDevice()->CreateImage(&imageCreateInfo, &image);
            if (Failed(ppxres)) {
                PPX_ASSERT_MSG(false, "image create failed");
                return ppxres;
            }

            mColorImages.push_back(image);
        }

        for (size_t i = 0; i < depthImages.size(); ++i) {
            grfx::ImageCreateInfo imageCreateInfo = grfx::ImageCreateInfo::DepthStencilTarget(pCreateInfo->width, pCreateInfo->height, pCreateInfo->depthFormat, grfx::SAMPLE_COUNT_1);
            imageCreateInfo.pApiObject            = (void*)(depthImages[i]);

            grfx::ImagePtr image;
            Result         ppxres = GetDevice()->CreateImage(&imageCreateInfo, &image);
            if (Failed(ppxres)) {
                PPX_ASSERT_MSG(false, "image create failed");
                return ppxres;
            }

            mDepthImages.push_back(image);
        }
    }

    return ppx::SUCCESS;
}

void XRSwapchain::DestroyApiObjects()
{
}

} // namespace vk
} // namespace grfx
} // namespace ppx

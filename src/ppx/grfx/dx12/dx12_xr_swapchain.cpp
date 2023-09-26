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

#include "ppx/grfx/dx12/dx12_xr_swapchain.h"
#include "ppx/grfx/dx12/dx12_device.h"
#include "ppx/grfx/dx12/dx12_image.h"
#include "ppx/grfx/dx12/dx12_instance.h"
#include "ppx/grfx/dx12/dx12_queue.h"
#include "ppx/grfx/dx12/dx12_sync.h"

namespace ppx {
namespace grfx {
namespace dx12 {

// -------------------------------------------------------------------------------------------------
// SurfaceSwapchain
// -------------------------------------------------------------------------------------------------
Result XRSwapchain::CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo)
{
    std::vector<ID3D12Resource*> colorImages;
    std::vector<ID3D12Resource*> depthImages;

    XrSwapchainCreateInfo info = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
    info.arraySize             = 1;
    info.mipCount              = 1;
    info.faceCount             = 1;
    info.format                = dx::ToDxgiFormat(pCreateInfo->colorFormat);
    info.width                 = pCreateInfo->width;
    info.height                = pCreateInfo->height;
    info.sampleCount           = pCreateInfo->sampleCount;
    info.usageFlags            = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
    CHECK_XR_CALL(xrCreateSwapchain(pCreateInfo->xrSession, &info, &mXrColorSwapchain));

    // Find out how many textures were generated for the swapchain.
    uint32_t imageCount = 0;
    CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrColorSwapchain, 0, &imageCount, nullptr));
    std::vector<XrSwapchainImageD3D12KHR> surfaceImages;
    surfaceImages.resize(imageCount, {XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR});
    CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrColorSwapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)surfaceImages.data()));
    for (uint32_t i = 0; i < imageCount; i++) {
        colorImages.push_back(surfaceImages[i].texture);
    }

    if (pCreateInfo->depthFormat != grfx::FORMAT_UNDEFINED) {
        XrSwapchainCreateInfo info = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
        info.arraySize             = 1;
        info.mipCount              = 1;
        info.faceCount             = 1;
        info.format                = dx::ToDxgiFormat(pCreateInfo->depthFormat);
        info.width                 = pCreateInfo->width;
        info.height                = pCreateInfo->height;
        info.sampleCount           = pCreateInfo->sampleCount;
        info.usageFlags            = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        CHECK_XR_CALL(xrCreateSwapchain(pCreateInfo->xrSession, &info, &mXrDepthSwapchain));

        imageCount = 0;
        CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrDepthSwapchain, 0, &imageCount, nullptr));
        std::vector<XrSwapchainImageD3D12KHR> swapchainDepthImages;
        swapchainDepthImages.resize(imageCount, {XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR});
        CHECK_XR_CALL(xrEnumerateSwapchainImages(mXrDepthSwapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)swapchainDepthImages.data()));
        for (uint32_t i = 0; i < imageCount; i++) {
            depthImages.push_back(swapchainDepthImages[i].texture);
        }

        PPX_ASSERT_MSG(depthImages.size() == colorImages.size(), "XR depth and color swapchains have different number of images");
    }

    // Create color images
    {
        auto ppxres = CreateColorImages(pCreateInfo->width, pCreateInfo->height, pCreateInfo->colorFormat, colorImages);
        if (Failed(ppxres)) {
            return ppxres;
        }
    }

    {
        for (size_t i = 0; i < depthImages.size(); ++i) {
            grfx::ImageCreateInfo imageCreateInfo = grfx::ImageCreateInfo::DepthStencilTarget(pCreateInfo->width, pCreateInfo->height, pCreateInfo->depthFormat, grfx::SAMPLE_COUNT_1);
            imageCreateInfo.pApiObject            = depthImages[i];

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

Result XRSwapchain::CreateColorImages(uint32_t width, uint32_t height, grfx::Format format, const std::vector<ID3D12Resource*>& colorImages)
{
    for (size_t i = 0; i < colorImages.size(); ++i) {
        grfx::ImageCreateInfo imageCreateInfo           = {};
        imageCreateInfo.type                            = grfx::IMAGE_TYPE_2D;
        imageCreateInfo.width                           = width;
        imageCreateInfo.height                          = height;
        imageCreateInfo.depth                           = 1;
        imageCreateInfo.format                          = format;
        imageCreateInfo.sampleCount                     = grfx::SAMPLE_COUNT_1;
        imageCreateInfo.mipLevelCount                   = 1;
        imageCreateInfo.arrayLayerCount                 = 1;
        imageCreateInfo.usageFlags.bits.transferSrc     = true;
        imageCreateInfo.usageFlags.bits.transferDst     = true;
        imageCreateInfo.usageFlags.bits.sampled         = true;
        imageCreateInfo.usageFlags.bits.storage         = true;
        imageCreateInfo.usageFlags.bits.colorAttachment = true;
        imageCreateInfo.pApiObject                      = colorImages[i];

        grfx::ImagePtr image;
        Result         ppxres = GetDevice()->CreateImage(&imageCreateInfo, &image);
        if (Failed(ppxres)) {
            PPX_ASSERT_MSG(false, "image create failed");
            return ppxres;
        }

        mColorImages.push_back(image);
    }

    return ppx::SUCCESS;
}

} // namespace dx12
} // namespace grfx
} // namespace ppx

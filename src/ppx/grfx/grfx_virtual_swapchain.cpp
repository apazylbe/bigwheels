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

#include "ppx/grfx/grfx_virtual_swapchain.h"
#include "ppx/grfx/grfx_device.h"
#include "ppx/grfx/grfx_render_pass.h"
#include "ppx/grfx/grfx_instance.h"

namespace ppx {
namespace grfx {

Result VirtualSwapchain::CreateInternal()
{
    PPX_ASSERT_NULL_ARG(mCreateInfo.pQueue);
    if (IsNull(mCreateInfo.pQueue)) {
        return ppx::ERROR_UNEXPECTED_NULL_ARGUMENT;
    }

    Result ppxres = ppx::SUCCESS;
    for (uint32_t i = 0; i < mCreateInfo.imageCount; ++i) {
        grfx::ImageCreateInfo rtCreateInfo = ImageCreateInfo::RenderTarget2D(mCreateInfo.width, mCreateInfo.height, mCreateInfo.colorFormat);
        rtCreateInfo.ownership             = grfx::OWNERSHIP_RESTRICTED;
        rtCreateInfo.RTVClearValue         = {0.0f, 0.0f, 0.0f, 0.0f};
        rtCreateInfo.initialState          = grfx::RESOURCE_STATE_PRESENT;
        rtCreateInfo.usageFlags =
            grfx::IMAGE_USAGE_COLOR_ATTACHMENT |
            grfx::IMAGE_USAGE_TRANSFER_SRC |
            grfx::IMAGE_USAGE_TRANSFER_DST |
            grfx::IMAGE_USAGE_SAMPLED;

        grfx::ImagePtr renderTarget;
        ppxres = GetDevice()->CreateImage(&rtCreateInfo, &renderTarget);
        if (Failed(ppxres)) {
            return ppxres;
        }

        mColorImages.push_back(renderTarget);
    }

    ppxres = CreateDepthImages();
    if (Failed(ppxres)) {
        return ppxres;
    }

    // Set mCurrentImageIndex to (imageCount - 1) so that the first
    // AcquireNextImage call acquires the first image at index 0.
    mCurrentImageIndex = mCreateInfo.imageCount - 1;

    // Create command buffers to signal and wait semaphores at
    // AcquireNextImage and Present calls.
    for (uint32_t i = 0; i < mCreateInfo.imageCount; ++i) {
        grfx::CommandBufferPtr commandBuffer = nullptr;
        mCreateInfo.pQueue->CreateCommandBuffer(&commandBuffer, 0, 0);
        mCommandBuffers.push_back(commandBuffer);
    }
    return ppx::SUCCESS;
}

void VirtualSwapchain::DestroyInternal()
{
    for (auto& elem : mCommandBuffers) {
        if (elem) {
            mCreateInfo.pQueue->DestroyCommandBuffer(elem);
        }
    }
    mCommandBuffers.clear();
}

Result VirtualSwapchain::AcquireNextImage(uint64_t timeout, grfx::Semaphore* pSemaphore, grfx::Fence* pFence, uint32_t* pImageIndex)
{
    *pImageIndex       = (mCurrentImageIndex + 1u) % CountU32(mColorImages);
    mCurrentImageIndex = *pImageIndex;

    grfx::CommandBufferPtr commandBuffer = mCommandBuffers[mCurrentImageIndex];

    commandBuffer->Begin();
    commandBuffer->End();

    grfx::SubmitInfo sInfo     = {};
    sInfo.ppCommandBuffers     = &commandBuffer;
    sInfo.commandBufferCount   = 1;
    sInfo.pFence               = pFence;
    sInfo.ppSignalSemaphores   = &pSemaphore;
    sInfo.signalSemaphoreCount = 1;
    mCreateInfo.pQueue->Submit(&sInfo);

    return ppx::SUCCESS;
}

Result VirtualSwapchain::Present(uint32_t imageIndex, uint32_t waitSemaphoreCount, const grfx::Semaphore* const* ppWaitSemaphores)
{
    grfx::CommandBufferPtr commandBuffer = mCommandBuffers[mCurrentImageIndex];

    commandBuffer->Begin();
    commandBuffer->End();

    grfx::SubmitInfo sInfo   = {};
    sInfo.ppCommandBuffers   = &commandBuffer;
    sInfo.commandBufferCount = 1;
    sInfo.ppWaitSemaphores   = ppWaitSemaphores;
    sInfo.waitSemaphoreCount = waitSemaphoreCount;
    mCreateInfo.pQueue->Submit(&sInfo);

    return ppx::SUCCESS;
}

Result VirtualSwapchain::CreateDepthImages()
{
    if (mCreateInfo.depthFormat == grfx::FORMAT_UNDEFINED) {
        return ppx::SUCCESS;
    }
    if (!mDepthImages.empty()) {
        PPX_ASSERT_MSG(false, "Depth images already exist");
        return ppx::ERROR_GRFX_OPERATION_NOT_PERMITTED;
    }

    for (uint32_t i = 0; i < mCreateInfo.imageCount; ++i) {
        grfx::ImageCreateInfo dpCreateInfo = ImageCreateInfo::DepthStencilTarget(mCreateInfo.width, mCreateInfo.height, mCreateInfo.depthFormat);
        dpCreateInfo.ownership             = grfx::OWNERSHIP_RESTRICTED;
        dpCreateInfo.DSVClearValue         = {1.0f, 0xFF};

        grfx::ImagePtr depthStencilTarget;
        auto           ppxres = GetDevice()->CreateImage(&dpCreateInfo, &depthStencilTarget);
        if (Failed(ppxres)) {
            return ppxres;
        }

        mDepthImages.push_back(depthStencilTarget);
    }

    return ppx::SUCCESS;
}

} // namespace grfx
} // namespace ppx

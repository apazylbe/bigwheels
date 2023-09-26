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

#include "ppx/grfx/grfx_device.h"
#include "ppx/grfx/grfx_surface_swapchain.h"

namespace ppx {
namespace grfx {

Result SurfaceSwapchain::CreateInternal()
{
    if (CountU32(mColorImages) != mCreateInfo.imageCount) {
        PPX_LOG_INFO("Swapchain actual image count is different from what was requested\n"
                     << "   actual    : " << CountU32(mColorImages) << "\n"
                     << "   requested : " << mCreateInfo.imageCount);
    }

    mCreateInfo.imageCount = CountU32(mColorImages);

    return CreateDepthImages();
}

void SurfaceSwapchain::DestroyInternal()
{
    // nothing to do.
}

Result SurfaceSwapchain::AcquireNextImage(
    uint64_t         timeout,    // Nanoseconds
    grfx::Semaphore* pSemaphore, // Wait sempahore
    grfx::Fence*     pFence,     // Wait fence
    uint32_t*        pImageIndex)
{
    return AcquireNextImageImpl(timeout, pSemaphore, pFence, pImageIndex);
}

Result SurfaceSwapchain::Present(
    uint32_t                      imageIndex,
    uint32_t                      waitSemaphoreCount,
    const grfx::Semaphore* const* ppWaitSemaphores)
{
    return PresentImpl(imageIndex, waitSemaphoreCount, ppWaitSemaphores);
}

Result SurfaceSwapchain::CreateDepthImages()
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

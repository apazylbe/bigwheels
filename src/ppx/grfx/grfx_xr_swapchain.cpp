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

#include "ppx/grfx/grfx_xr_swapchain.h"

namespace ppx {
namespace grfx {

Result XRSwapchain::CreateInternal()
{
    mCreateInfo.imageCount = CountU32(mColorImages);
    return ppx::SUCCESS;
}

void XRSwapchain::DestroyInternal()
{
    if (mXrColorSwapchain != XR_NULL_HANDLE) {
        xrDestroySwapchain(mXrColorSwapchain);
    }
    if (mXrDepthSwapchain != XR_NULL_HANDLE) {
        xrDestroySwapchain(mXrDepthSwapchain);
    }
}

Result XRSwapchain::AcquireNextImage(
    uint64_t         timeout,    // Nanoseconds
    grfx::Semaphore* pSemaphore, // Wait sempahore
    grfx::Fence*     pFence,     // Wait fence
    uint32_t*        pImageIndex)
{
    PPX_ASSERT_MSG(mXrColorSwapchain != XR_NULL_HANDLE, "Invalid color xrSwapchain handle!");
    PPX_ASSERT_MSG(pSemaphore == nullptr, "Should not use semaphore when XR is enabled!");
    PPX_ASSERT_MSG(pFence == nullptr, "Should not use fence when XR is enabled!");
    XrSwapchainImageAcquireInfo acquire_info = {XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    CHECK_XR_CALL(xrAcquireSwapchainImage(mXrColorSwapchain, &acquire_info, pImageIndex));

    XrSwapchainImageWaitInfo wait_info = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    wait_info.timeout                  = XR_INFINITE_DURATION;
    CHECK_XR_CALL(xrWaitSwapchainImage(mXrColorSwapchain, &wait_info));

    if (mXrDepthSwapchain != XR_NULL_HANDLE) {
        uint32_t                    colorImageIndex = *pImageIndex;
        XrSwapchainImageAcquireInfo acquire_info    = {XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        CHECK_XR_CALL(xrAcquireSwapchainImage(mXrDepthSwapchain, &acquire_info, pImageIndex));

        XrSwapchainImageWaitInfo wait_info = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        wait_info.timeout                  = XR_INFINITE_DURATION;
        CHECK_XR_CALL(xrWaitSwapchainImage(mXrDepthSwapchain, &wait_info));

        PPX_ASSERT_MSG(colorImageIndex == *pImageIndex, "Color and depth swapchain image indices are different");
    }
    mCurrentImageIndex = *pImageIndex;
    return ppx::SUCCESS;
}

Result XRSwapchain::Present(
    uint32_t                      imageIndex,
    uint32_t                      waitSemaphoreCount,
    const grfx::Semaphore* const* ppWaitSemaphores)
{
    return ppx::SUCCESS;
}

} // namespace grfx
} // namespace ppx

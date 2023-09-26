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

#ifndef ppx_grfx_xr_swapchain_h
#define ppx_grfx_xr_swapchain_h

#include "ppx/grfx/grfx_config.h"
#include "ppx/grfx/grfx_swapchain.h"
#include "ppx/xr_component.h"

namespace ppx {
namespace grfx {

#if defined(PPX_BUILD_XR)

class XRSwapchain : public Swapchain
{
public:
    virtual ~XRSwapchain(){};
    virtual SwapchainType GetType() const override { return grfx::SwapchainType::SWAPCHAIN_TYPE_XR; }
    virtual Result        AcquireNextImage(
               uint64_t         timeout,    // Nanoseconds
               grfx::Semaphore* pSemaphore, // Wait sempahore
               grfx::Fence*     pFence,     // Wait fence
               uint32_t*        pImageIndex) override;
    virtual Result Present(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) override;

    virtual bool ShouldSkipExternalSynchronization() const override
    {
        return true;
    }

    XrSwapchain GetXrColorSwapchain() const
    {
        return mXrColorSwapchain;
    }
    XrSwapchain GetXrDepthSwapchain() const
    {
        return mXrDepthSwapchain;
    }

protected:
    virtual Result CreateInternal() override;
    virtual void   DestroyInternal() override;
    XrSwapchain    mXrColorSwapchain = XR_NULL_HANDLE;
    XrSwapchain    mXrDepthSwapchain = XR_NULL_HANDLE;
};

#else

class XRSwapchain : public Swapchain
{
public:
    virtual ~XRSwapchain(){};
    virtual SwapchainType GetType() const override { return grfx::SwapchainType::SWAPCHAIN_TYPE_XR; }
    virtual Result        AcquireNextImage(
               uint64_t         timeout,    // Nanoseconds
               grfx::Semaphore* pSemaphore, // Wait sempahore
               grfx::Fence*     pFence,     // Wait fence
               uint32_t*        pImageIndex) override
    {
        return ppx::SUCCESS;
    }
    virtual Result Present(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) override { return ppx::SUCCESS; }

protected:
    virtual Result CreateInternal() override { PPX_ASSERT_MSG(false, "XR support is not active"); }
    virtual void   DestroyInternal() override {}
};

#endif

} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_xr_swapchain_h

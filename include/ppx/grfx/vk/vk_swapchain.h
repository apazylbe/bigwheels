// Copyright 2022 Google LLC
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

#ifndef ppx_grfx_vk_swapchain_h
#define ppx_grfx_vk_swapchain_h

#include "ppx/grfx/vk/vk_config.h"
#include "ppx/grfx/grfx_swapchain.h"

namespace ppx {
namespace grfx {
namespace vk {

//! @class Surface
//!
//!
class Surface
    : public grfx::Surface
{
public:
    Surface() {}
    virtual ~Surface() {}

    VkSurfacePtr GetVkSurface() const { return mSurface; }

    VkSurfaceCapabilitiesKHR              GetCapabilities() const;
    const std::vector<VkSurfaceFormatKHR> GetSurfaceFormats() const { return mSurfaceFormats; }

    virtual uint32_t GetMinImageWidth() const override;
    virtual uint32_t GetMinImageHeight() const override;
    virtual uint32_t GetMinImageCount() const override;
    virtual uint32_t GetMaxImageWidth() const override;
    virtual uint32_t GetMaxImageHeight() const override;
    virtual uint32_t GetMaxImageCount() const override;

    virtual uint32_t GetCurrentImageWidth() const override;
    virtual uint32_t GetCurrentImageHeight() const override;

protected:
    virtual Result CreateApiObjects(const grfx::SurfaceCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;

private:
    VkSurfacePtr                    mSurface;
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
    std::vector<uint32_t>           mPresentableQueueFamilies;
    std::vector<VkPresentModeKHR>   mPresentModes;
};

// -------------------------------------------------------------------------------------------------

//! @class Swapchain
//!
//!
class SurfaceSwapchain
    : public grfx::SurfaceSwapchain
{
public:
    SurfaceSwapchain() {}
    virtual ~SurfaceSwapchain() {}

    VkSwapchainPtr GetVkSwapchain() const { return mSwapchain; }

protected:
        virtual Result        AcquireNextImageImpl(
               uint64_t         timeout,    // Nanoseconds
               grfx::Semaphore* pSemaphore, // Wait sempahore
               grfx::Fence*     pFence,     // Wait fence
               uint32_t*        pImageIndex) override;

    virtual Result PresentImpl(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) override;
    virtual Result Resize(uint32_t width, uint32_t height) override { return ppx::ERROR_FAILED; }
    
    virtual Result CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;

private:
    VkSwapchainPtr mSwapchain;
    VkQueuePtr     mQueue;
};

class XRSwapchain : public grfx::XRSwapchain
{
public:
    XRSwapchain() {}
    virtual ~XRSwapchain() {}

    virtual Result Resize(uint32_t width, uint32_t height) override { return ppx::ERROR_FAILED; }

protected:
    virtual Result CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;

private:
    VkQueuePtr mQueue;
};

} // namespace vk
} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_vk_swapchain_h

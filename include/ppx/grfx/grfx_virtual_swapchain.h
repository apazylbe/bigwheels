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

#ifndef ppx_grfx_virtual_swapchain_h
#define ppx_grfx_virtual_swapchain_h

#include "ppx/grfx/grfx_config.h"
#include "ppx/grfx/grfx_swapchain.h"

namespace ppx {
namespace grfx {

class VirtualSwapchain : public Swapchain
{
public:
    virtual ~VirtualSwapchain(){};
    virtual SwapchainType GetType() const override { return grfx::SwapchainType::SWAPCHAIN_TYPE_VIRTUAL; }
    virtual Result        AcquireNextImage(
               uint64_t         timeout,    // Nanoseconds
               grfx::Semaphore* pSemaphore, // Wait sempahore
               grfx::Fence*     pFence,     // Wait fence
               uint32_t*        pImageIndex) override;

    virtual Result Present(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) override;

    virtual Result Resize(uint32_t width, uint32_t height) override { return ppx::ERROR_FAILED; }

protected:
    virtual Result CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo) override { return ppx::SUCCESS; }
    virtual void   DestroyApiObjects() override {}

    virtual Result CreateInternal() override;
    virtual void   DestroyInternal() override;

private:
    Result CreateDepthImages();

    std::vector<grfx::CommandBufferPtr> mCommandBuffers;
};

} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_virtual_swapchain_h

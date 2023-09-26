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

#ifndef ppx_grfx_vk_xr_swapchain_h
#define ppx_grfx_vk_xr_swapchain_h

#include "ppx/grfx/vk/vk_config.h"
#include "ppx/grfx/grfx_xr_swapchain.h"

namespace ppx {
namespace grfx {
namespace vk {

#if defined(PPX_BUILD_XR)

class XRSwapchain : public grfx::XRSwapchain
{
public:
    XRSwapchain() {}
    virtual ~XRSwapchain() {}

    virtual Result Resize(uint32_t width, uint32_t height) override { return ppx::ERROR_FAILED; }

protected:
    virtual Result CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;
};

#else

class XRSwapchain : public grfx::XRSwapchain
{
public:
    XRSwapchain() {}
    virtual ~XRSwapchain() {}

    virtual Result Resize(uint32_t width, uint32_t height) override { return ppx::ERROR_FAILED; }

protected:
    virtual Result CreateApiObjects(const grfx::SwapchainCreateInfo* pCreateInfo) override { return ppx::SUCCESS; }
    virtual void   DestroyApiObjects() override {}
};

#endif

} // namespace vk
} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_vk_xr_swapchain_h

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

#ifndef ppx_grfx_surface_swapchain_h
#define ppx_grfx_surface_swapchain_h

#include "ppx/grfx/grfx_config.h"
#include "ppx/grfx/grfx_swapchain.h"

// clang-format off
#if defined(PPX_LINUX_XCB)
#   include <xcb/xcb.h>
#elif defined(PPX_LINUX_XLIB)
#   include <X11/Xlib.h>
#elif defined(PPX_LINUX_WAYLAND)
#   include <wayland-client.h>
#elif defined(PPX_MSW)
#   include <Windows.h>
#elif defined(PPX_ANDROID)
#   include <android_native_app_glue.h>
#endif
// clang-format on

namespace ppx {
namespace grfx {

// SurfaceCreateInfo
struct SurfaceCreateInfo
{
    // clang-format off
    grfx::Gpu*            pGpu = nullptr;
#if defined(PPX_LINUX_WAYLAND)
    struct wl_display*    display;
    struct wl_surface*    surface;
#elif defined(PPX_LINUX_XCB)
    xcb_connection_t*     connection;
    xcb_window_t          window;
#elif defined(PPX_LINUX_XLIB)
    Display*              dpy;
    Window                window;
#elif defined(PPX_MSW)
    HINSTANCE             hinstance;
    HWND                  hwnd;
#elif defined(PPX_ANDROID)
    android_app*          androidAppContext;
#endif
    // clang-format on
};

// Surface
class Surface
    : public grfx::InstanceObject<grfx::SurfaceCreateInfo>
{
public:
    Surface() {}
    virtual ~Surface() {}

    virtual uint32_t GetMinImageWidth() const  = 0;
    virtual uint32_t GetMinImageHeight() const = 0;
    virtual uint32_t GetMinImageCount() const  = 0;
    virtual uint32_t GetMaxImageWidth() const  = 0;
    virtual uint32_t GetMaxImageHeight() const = 0;
    virtual uint32_t GetMaxImageCount() const  = 0;

    static constexpr uint32_t kInvalidExtent = std::numeric_limits<uint32_t>::max();

    virtual uint32_t GetCurrentImageWidth() const { return kInvalidExtent; }
    virtual uint32_t GetCurrentImageHeight() const { return kInvalidExtent; }
};

// SurfaceSwapchain - a swapchain that is backed by a surface.
class SurfaceSwapchain : public Swapchain
{
public:
    virtual ~SurfaceSwapchain(){};
    virtual SwapchainType GetType() const override { return grfx::SwapchainType::SWAPCHAIN_TYPE_SURFACE; }
    virtual Result        AcquireNextImage(
               uint64_t         timeout,    // Nanoseconds
               grfx::Semaphore* pSemaphore, // Wait sempahore
               grfx::Fence*     pFence,     // Wait fence
               uint32_t*        pImageIndex) override;

    virtual Result Present(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) override;

protected:
    virtual Result CreateInternal() override;
    virtual void   DestroyInternal() override;

    // The following two functions have API specific implementations.
    virtual Result AcquireNextImageImpl(
        uint64_t         timeout,    // Nanoseconds
        grfx::Semaphore* pSemaphore, // Wait sempahore
        grfx::Fence*     pFence,     // Wait fence
        uint32_t*        pImageIndex) = 0;

    virtual Result PresentImpl(
        uint32_t                      imageIndex,
        uint32_t                      waitSemaphoreCount,
        const grfx::Semaphore* const* ppWaitSemaphores) = 0;

private:
    Result CreateDepthImages();
};

} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_surface_swapchain_h

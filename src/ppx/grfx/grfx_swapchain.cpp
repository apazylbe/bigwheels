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

#include "ppx/grfx/grfx_swapchain.h"
#include "ppx/grfx/grfx_device.h"
#include "ppx/grfx/grfx_render_pass.h"
#include "ppx/grfx/grfx_instance.h"

namespace ppx {
namespace grfx {

Result Swapchain::Create(const grfx::SwapchainCreateInfo* pCreateInfo)
{
    Result ppxres = grfx::DeviceObject<grfx::SwapchainCreateInfo>::Create(pCreateInfo);
    if (Failed(ppxres)) {
        return ppxres;
    }

    //
    // NOTE: mCreateInfo will be used from this point on.
    //

    ppxres = CreateInternal();
    if (Failed(ppxres)) {
        return ppxres;
    }

    ppxres = CreateRenderTargets();
    if (Failed(ppxres)) {
        return ppxres;
    }

    ppxres = CreateRenderPasses();
    if (Failed(ppxres)) {
        return ppxres;
    }

    PPX_LOG_INFO("Swapchain created");
    PPX_LOG_INFO("   "
                 << "resolution  : " << mCreateInfo.width << "x" << mCreateInfo.height);
    PPX_LOG_INFO("   "
                 << "image count : " << mCreateInfo.imageCount);

    return ppx::SUCCESS;
}

void Swapchain::Destroy()
{
    DestroyRenderPasses();

    DestroyRenderTargets();

    DestroyDepthImages();

    DestroyColorImages();

    DestroyInternal();

    grfx::DeviceObject<grfx::SwapchainCreateInfo>::Destroy();
}

void Swapchain::DestroyColorImages()
{
    for (auto& elem : mColorImages) {
        if (elem) {
            GetDevice()->DestroyImage(elem);
        }
    }
    mColorImages.clear();
}

void Swapchain::DestroyDepthImages()
{
    for (auto& elem : mDepthImages) {
        if (elem) {
            GetDevice()->DestroyImage(elem);
        }
    }
    mDepthImages.clear();
}

Result Swapchain::CreateRenderTargets()
{
    uint32_t imageCount = CountU32(mColorImages);
    PPX_ASSERT_MSG((imageCount > 0), "No color images found for swapchain renderpasses");
    for (size_t i = 0; i < imageCount; ++i) {
        auto                             imagePtr      = mColorImages[i];
        grfx::RenderTargetViewCreateInfo rtvCreateInfo = grfx::RenderTargetViewCreateInfo::GuessFromImage(imagePtr);
        rtvCreateInfo.loadOp                           = ppx::grfx::ATTACHMENT_LOAD_OP_CLEAR;
        rtvCreateInfo.ownership                        = grfx::OWNERSHIP_RESTRICTED;

        grfx::RenderTargetViewPtr rtv;
        Result                    ppxres = GetDevice()->CreateRenderTargetView(&rtvCreateInfo, &rtv);
        if (Failed(ppxres)) {
            PPX_ASSERT_MSG(false, "grfx::Swapchain::CreateRenderTargets() for LOAD_OP_CLEAR failed");
            return ppxres;
        }
        mClearRenderTargets.push_back(rtv);

        rtvCreateInfo.loadOp = ppx::grfx::ATTACHMENT_LOAD_OP_LOAD;
        ppxres               = GetDevice()->CreateRenderTargetView(&rtvCreateInfo, &rtv);
        if (Failed(ppxres)) {
            PPX_ASSERT_MSG(false, "grfx::Swapchain::CreateRenderTargets() for LOAD_OP_LOAD failed");
            return ppxres;
        }
        mLoadRenderTargets.push_back(rtv);

        if (!mDepthImages.empty()) {
            auto                             depthImage    = mDepthImages[i];
            grfx::DepthStencilViewCreateInfo dsvCreateInfo = grfx::DepthStencilViewCreateInfo::GuessFromImage(depthImage);
            dsvCreateInfo.depthLoadOp                      = ppx::grfx::ATTACHMENT_LOAD_OP_CLEAR;
            dsvCreateInfo.stencilLoadOp                    = ppx::grfx::ATTACHMENT_LOAD_OP_CLEAR;
            dsvCreateInfo.ownership                        = ppx::grfx::OWNERSHIP_RESTRICTED;

            grfx::DepthStencilViewPtr dsv;
            ppxres = GetDevice()->CreateDepthStencilView(&dsvCreateInfo, &dsv);
            if (Failed(ppxres)) {
                PPX_ASSERT_MSG(false, "grfx::Swapchain::CreateRenderTargets() for depth stencil view failed");
                return ppxres;
            }

            mDepthStencilViews.push_back(dsv);
        }
    }

    return ppx::SUCCESS;
}

Result Swapchain::CreateRenderPasses()
{
    uint32_t imageCount = CountU32(mColorImages);
    PPX_ASSERT_MSG((imageCount > 0), "No color images found for swapchain renderpasses");

    // Create render passes with grfx::ATTACHMENT_LOAD_OP_CLEAR for render target.
    for (size_t i = 0; i < imageCount; ++i) {
        grfx::RenderPassCreateInfo rpCreateInfo  = {};
        rpCreateInfo.width                       = mCreateInfo.width;
        rpCreateInfo.height                      = mCreateInfo.height;
        rpCreateInfo.renderTargetCount           = 1;
        rpCreateInfo.pRenderTargetViews[0]       = mClearRenderTargets[i];
        rpCreateInfo.pDepthStencilView           = mDepthImages.empty() ? nullptr : mDepthStencilViews[i];
        rpCreateInfo.renderTargetClearValues[0]  = {{0.0f, 0.0f, 0.0f, 0.0f}};
        rpCreateInfo.depthStencilClearValue      = {1.0f, 0xFF};
        rpCreateInfo.ownership                   = grfx::OWNERSHIP_RESTRICTED;

        grfx::RenderPassPtr renderPass;
        auto                ppxres = GetDevice()->CreateRenderPass(&rpCreateInfo, &renderPass);
        if (Failed(ppxres)) {
            PPX_ASSERT_MSG(false, "grfx::Swapchain::CreateRenderPass(CLEAR) failed");
            return ppxres;
        }

        mClearRenderPasses.push_back(renderPass);
    }

    // Create render passes with grfx::ATTACHMENT_LOAD_OP_LOAD for render target.
    for (size_t i = 0; i < imageCount; ++i) {
        grfx::RenderPassCreateInfo rpCreateInfo  = {};
        rpCreateInfo.width                       = mCreateInfo.width;
        rpCreateInfo.height                      = mCreateInfo.height;
        rpCreateInfo.renderTargetCount           = 1;
        rpCreateInfo.pRenderTargetViews[0]       = mLoadRenderTargets[i];
        rpCreateInfo.pDepthStencilView           = mDepthImages.empty() ? nullptr : mDepthStencilViews[i];
        rpCreateInfo.renderTargetClearValues[0]  = {{0.0f, 0.0f, 0.0f, 0.0f}};
        rpCreateInfo.depthStencilClearValue      = {1.0f, 0xFF};
        rpCreateInfo.ownership                   = grfx::OWNERSHIP_RESTRICTED;

        grfx::RenderPassPtr renderPass;
        auto                ppxres = GetDevice()->CreateRenderPass(&rpCreateInfo, &renderPass);
        if (Failed(ppxres)) {
            PPX_ASSERT_MSG(false, "grfx::Swapchain::CreateRenderPass(LOAD) failed");
            return ppxres;
        }

        mLoadRenderPasses.push_back(renderPass);
    }

    return ppx::SUCCESS;
}

void Swapchain::DestroyRenderTargets()
{
    for (auto& rtv : mClearRenderTargets) {
        GetDevice()->DestroyRenderTargetView(rtv);
    }
    mClearRenderTargets.clear();
    for (auto& rtv : mLoadRenderTargets) {
        GetDevice()->DestroyRenderTargetView(rtv);
    }
    mLoadRenderTargets.clear();
    for (auto& rtv : mDepthStencilViews) {
        GetDevice()->DestroyDepthStencilView(rtv);
    }
    mDepthStencilViews.clear();
}

void Swapchain::DestroyRenderPasses()
{
    for (auto& elem : mClearRenderPasses) {
        if (elem) {
            GetDevice()->DestroyRenderPass(elem);
        }
    }
    mClearRenderPasses.clear();

    for (auto& elem : mLoadRenderPasses) {
        if (elem) {
            GetDevice()->DestroyRenderPass(elem);
        }
    }
    mLoadRenderPasses.clear();
}

Result Swapchain::GetColorImage(uint32_t imageIndex, grfx::Image** ppImage) const
{
    if (!IsIndexInRange(imageIndex, mColorImages)) {
        return ppx::ERROR_OUT_OF_RANGE;
    }
    *ppImage = mColorImages[imageIndex];
    return ppx::SUCCESS;
}

Result Swapchain::GetDepthImage(uint32_t imageIndex, grfx::Image** ppImage) const
{
    if (!IsIndexInRange(imageIndex, mDepthImages)) {
        return ppx::ERROR_OUT_OF_RANGE;
    }
    *ppImage = mDepthImages[imageIndex];
    return ppx::SUCCESS;
}

Result Swapchain::GetRenderPass(uint32_t imageIndex, grfx::AttachmentLoadOp loadOp, grfx::RenderPass** ppRenderPass) const
{
    if (!IsIndexInRange(imageIndex, mClearRenderPasses)) {
        return ppx::ERROR_OUT_OF_RANGE;
    }
    if (loadOp == grfx::ATTACHMENT_LOAD_OP_CLEAR) {
        *ppRenderPass = mClearRenderPasses[imageIndex];
    }
    else {
        *ppRenderPass = mLoadRenderPasses[imageIndex];
    }
    return ppx::SUCCESS;
}

Result Swapchain::GetRenderTargetView(uint32_t imageIndex, grfx::AttachmentLoadOp loadOp, grfx::RenderTargetView** ppView) const
{
    if (!IsIndexInRange(imageIndex, mClearRenderTargets)) {
        return ppx::ERROR_OUT_OF_RANGE;
    }
    if (loadOp == grfx::ATTACHMENT_LOAD_OP_CLEAR) {
        *ppView = mClearRenderTargets[imageIndex];
    }
    else {
        *ppView = mLoadRenderTargets[imageIndex];
    }
    return ppx::SUCCESS;
}

Result Swapchain::GetDepthStencilView(uint32_t imageIndex, grfx::DepthStencilView** ppView) const
{
    if (!IsIndexInRange(imageIndex, mDepthStencilViews)) {
        return ppx::ERROR_OUT_OF_RANGE;
    }
    *ppView = mDepthStencilViews[imageIndex];
    return ppx::SUCCESS;
}

grfx::ImagePtr Swapchain::GetColorImage(uint32_t imageIndex) const
{
    grfx::ImagePtr object;
    GetColorImage(imageIndex, &object);
    return object;
}

grfx::ImagePtr Swapchain::GetDepthImage(uint32_t imageIndex) const
{
    grfx::ImagePtr object;
    GetDepthImage(imageIndex, &object);
    return object;
}

grfx::RenderPassPtr Swapchain::GetRenderPass(uint32_t imageIndex, grfx::AttachmentLoadOp loadOp) const
{
    grfx::RenderPassPtr object;
    GetRenderPass(imageIndex, loadOp, &object);
    return object;
}

grfx::RenderTargetViewPtr Swapchain::GetRenderTargetView(uint32_t imageIndex, grfx::AttachmentLoadOp loadOp) const
{
    grfx::RenderTargetViewPtr object;
    GetRenderTargetView(imageIndex, loadOp, &object);
    return object;
}

grfx::DepthStencilViewPtr Swapchain::GetDepthStencilView(uint32_t imageIndex) const
{
    grfx::DepthStencilViewPtr object;
    GetDepthStencilView(imageIndex, &object);
    return object;
}

} // namespace grfx
} // namespace ppx

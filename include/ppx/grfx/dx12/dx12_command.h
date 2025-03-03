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

#ifndef ppx_grfx_dx12_command_buffer_h
#define ppx_grfx_dx12_command_buffer_h

#include "ppx/grfx/dx12/dx12_config.h"
#include "ppx/grfx/grfx_command.h"

namespace ppx {
namespace grfx {
namespace dx12 {

class CommandBuffer
    : public grfx::CommandBuffer
{
public:
    CommandBuffer() {}
    virtual ~CommandBuffer() {}

    typename D3D12GraphicsCommandListPtr::InterfaceType* GetDxCommandList() const { return mCommandList.Get(); }

    virtual Result Begin() override;
    virtual Result End() override;

private:
    virtual void BeginRenderPassImpl(const grfx::RenderPassBeginInfo* pBeginInfo) override;
    virtual void EndRenderPassImpl() override;

    virtual void BeginRenderingImpl(const grfx::RenderingInfo* pRenderingInfo) override;
    virtual void EndRenderingImpl() override;

    virtual void PushDescriptorImpl(
        grfx::CommandType              pipelineBindPoint,
        const grfx::PipelineInterface* pInterface,
        grfx::DescriptorType           descriptorType,
        uint32_t                       binding,
        uint32_t                       set,
        uint32_t                       bufferOffset,
        const grfx::Buffer*            pBuffer,
        const grfx::SampledImageView*  pSampledImageView,
        const grfx::StorageImageView*  pStorageImageView,
        const grfx::Sampler*           pSampler) override;

public:
    virtual void ClearRenderTarget(
        grfx::Image*                        pImage,
        const grfx::RenderTargetClearValue& clearValue) override;
    virtual void ClearDepthStencil(
        grfx::Image*                        pImage,
        const grfx::DepthStencilClearValue& clearValue,
        uint32_t                            clearFlags) override;

    virtual void TransitionImageLayout(
        const grfx::Image*  pImage,
        uint32_t            mipLevel,
        uint32_t            mipLevelCount,
        uint32_t            arrayLayer,
        uint32_t            arrayLayerCount,
        grfx::ResourceState beforeState,
        grfx::ResourceState afterState,
        const grfx::Queue*  pSrcQueue,
        const grfx::Queue*  pDstQueue) override;

    virtual void BufferResourceBarrier(
        const grfx::Buffer* pBuffer,
        grfx::ResourceState beforeState,
        grfx::ResourceState afterState,
        const grfx::Queue*  pSrcQueue = nullptr,
        const grfx::Queue*  pDstQueue = nullptr) override;

    virtual void SetViewports(
        uint32_t              viewportCount,
        const grfx::Viewport* pViewports) override;

    virtual void SetScissors(
        uint32_t          scissorCount,
        const grfx::Rect* pScissors) override;

    virtual void BindGraphicsDescriptorSets(
        const grfx::PipelineInterface*    pInterface,
        uint32_t                          setCount,
        const grfx::DescriptorSet* const* ppSets) override;

    virtual void PushGraphicsConstants(
        const grfx::PipelineInterface* pInterface,
        uint32_t                       count,
        const void*                    pValues,
        uint32_t                       dstOffset) override;

    virtual void BindGraphicsPipeline(const grfx::GraphicsPipeline* pPipeline) override;

    virtual void BindComputeDescriptorSets(
        const grfx::PipelineInterface*    pInterface,
        uint32_t                          setCount,
        const grfx::DescriptorSet* const* ppSets) override;

    virtual void PushComputeConstants(
        const grfx::PipelineInterface* pInterface,
        uint32_t                       count,
        const void*                    pValues,
        uint32_t                       dstOffset) override;

    virtual void BindComputePipeline(const grfx::ComputePipeline* pPipeline) override;

    virtual void BindIndexBuffer(const grfx::IndexBufferView* pView) override;

    virtual void BindVertexBuffers(
        uint32_t                      viewCount,
        const grfx::VertexBufferView* pViews) override;

    virtual void Draw(
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance) override;

    virtual void DrawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t  vertexOffset,
        uint32_t firstInstance) override;

    virtual void Dispatch(
        uint32_t groupCountX,
        uint32_t groupCountY,
        uint32_t groupCountZ) override;

    virtual void CopyBufferToBuffer(
        const grfx::BufferToBufferCopyInfo* pCopyInfo,
        grfx::Buffer*                       pSrcBuffer,
        grfx::Buffer*                       pDstBuffer) override;

    virtual void CopyBufferToImage(
        const std::vector<grfx::BufferToImageCopyInfo>& pCopyInfos,
        grfx::Buffer*                                   pSrcBuffer,
        grfx::Image*                                    pDstImage) override;

    virtual void CopyBufferToImage(
        const grfx::BufferToImageCopyInfo* pCopyInfo,
        grfx::Buffer*                      pSrcBuffer,
        grfx::Image*                       pDstImage) override;

    virtual grfx::ImageToBufferOutputPitch CopyImageToBuffer(
        const grfx::ImageToBufferCopyInfo* pCopyInfo,
        grfx::Image*                       pSrcImage,
        grfx::Buffer*                      pDstBufferh) override;

    virtual void CopyImageToImage(
        const grfx::ImageToImageCopyInfo* pCopyInfo,
        grfx::Image*                      pSrcImage,
        grfx::Image*                      pDstImage) override;

    virtual void BeginQuery(
        const grfx::Query* pQuery,
        uint32_t           queryIndex) override;

    virtual void EndQuery(
        const grfx::Query* pQuery,
        uint32_t           queryIndex) override;

    virtual void WriteTimestamp(
        const grfx::Query*  pQuery,
        grfx::PipelineStage pipelineStage,
        uint32_t            queryIndex) override;

    virtual void ResolveQueryData(
        grfx::Query* pQuery,
        uint32_t     startIndex,
        uint32_t     numQueries) override;

protected:
    virtual Result CreateApiObjects(const grfx::internal::CommandBufferCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;

private:
    void SetGraphicsPipelineInterface(const grfx::PipelineInterface* pInterface);
    void SetComputePipelineInterface(const grfx::PipelineInterface* pInterface);

    void BindDescriptorSets(
        const grfx::PipelineInterface*    pInterface,
        uint32_t                          setCount,
        const grfx::DescriptorSet* const* ppSets,
        size_t&                           rdtCountCBVSRVUAV,
        size_t&                           rdtCountSampler);

private:
    D3D12GraphicsCommandListPtr    mCommandList;
    D3D12CommandAllocatorPtr       mCommandAllocator;
    UINT                           mHeapSizeCBVSRVUAV = 0;
    UINT                           mHeapSizeSampler   = 0;
    D3D12DescriptorHeapPtr         mHeapCBVSRVUAV;
    D3D12DescriptorHeapPtr         mHeapSampler;
    UINT                           mHeapOffsetCBVSRVUAV      = 0;
    UINT                           mHeapOffsetSampler        = 0;
    const grfx::PipelineInterface* mCurrentGraphicsInterface = nullptr;
    const grfx::PipelineInterface* mCurrentComputeInterface  = nullptr;

    struct RootDescriptorTable
    {
        UINT                        parameterIndex = PPX_VALUE_IGNORED;
        D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor = {~0ULL};
    };

    std::vector<RootDescriptorTable> mRootDescriptorTablesCBVSRVUAV;
    std::vector<RootDescriptorTable> mRootDescriptorTablesSampler;
};

// -------------------------------------------------------------------------------------------------

class CommandPool
    : public grfx::CommandPool
{
public:
    CommandPool() {}
    virtual ~CommandPool() {}

    // typename D3D12CommandAllocatorPtr::InterfaceType* GetDxCommandAllocator() const { return mCommandAllocator.Get(); }
    D3D12_COMMAND_LIST_TYPE GetDxCommandType() const { return mCommandType; }

protected:
    virtual Result CreateApiObjects(const grfx::CommandPoolCreateInfo* pCreateInfo) override;
    virtual void   DestroyApiObjects() override;

private:
    // D3D12CommandAllocatorPtr mCommandAllocator;
    D3D12_COMMAND_LIST_TYPE mCommandType = ppx::InvalidValue<D3D12_COMMAND_LIST_TYPE>();
};

} // namespace dx12
} // namespace grfx
} // namespace ppx

#endif // ppx_grfx_dx12_command_buffer_h

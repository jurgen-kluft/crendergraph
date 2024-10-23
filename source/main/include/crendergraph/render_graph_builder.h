#ifndef __CRED_RENDER_GRAPH_BUILDER_H__
#define __CRED_RENDER_GRAPH_BUILDER_H__

#include "crendergraph/render_graph.h"

namespace ncore
{

    enum class RGBuilderFlag
    {
        None,
        ShaderStagePS,
        ShaderStageNonPS,
    };

    class RGBuilder
    {
    public:
        RGBuilder(RenderGraph* graph, RenderGraphPassBase* pass)
        {
            m_pGraph = graph;
            m_pPass  = pass;
        }

        void SkipCulling() { m_pPass->MakeTarget(); }

        template <typename Resource> RGHandle Create(const typename Resource::Desc& desc, const nstring::str_t const* name) { return m_pGraph->Create<Resource>(desc, name); }

        RGHandle Import(IGfxTexture* texture, ngfx::GfxAccess::Flags state) { return m_pGraph->Import(texture, state); }

        RGHandle Read(const RGHandle& input, ngfx::GfxAccess::Flags usage, u32 subresource)
        {
            ASSERT(usage & (GfxAccessMaskSRV | GfxAccessIndirectArgs | GfxAccessCopySrc));

            ASSERT(GFX_ALL_SUB_RESOURCE != subresource); // RG doesn't support GFX_ALL_SUB_RESOURCE currently

            return m_pGraph->Read(m_pPass, input, usage, subresource);
        }

        RGHandle Read(const RGHandle& input, u32 subresource = 0, RGBuilderFlag flag = RGBuilderFlag::None)
        {
            ngfx::GfxAccess::Flags state;

            switch (m_pPass->GetType())
            {
                case RenderPassType::Graphics:
                    if (flag == RGBuilderFlag::ShaderStagePS)
                    {
                        state = ngfx::GfxAccess::PixelShaderSRV;
                    }
                    else if (flag == RGBuilderFlag::ShaderStageNonPS)
                    {
                        state = ngfx::GfxAccess::VertexShaderSRV;
                    }
                    else
                    {
                        state = ngfx::GfxAccess::PixelShaderSRV | ngfx::GfxAccess::VertexShaderSRV;
                    }
                    break;
                case RenderPassType::Compute:
                case RenderPassType::AsyncCompute: state = ngfx::GfxAccess::ComputeSRV; break;
                case RenderPassType::Copy: state = ngfx::GfxAccess::CopySrc; break;
                default: ASSERT(false); break;
            }
            return Read(input, state, subresource);
        }

        RGHandle ReadIndirectArg(const RGHandle& input, u32 subresource = 0) { return Read(input, ngfx::GfxAccess::IndirectArgs, subresource); }

        RGHandle Write(const RGHandle& input, ngfx::GfxAccess::Flags usage, u32 subresource)
        {
            ASSERT(usage & (GfxAccessMaskUAV | GfxAccessCopyDst));

            ASSERT(GFX_ALL_SUB_RESOURCE != subresource); // RG doesn't support GFX_ALL_SUB_RESOURCE currently

            return m_pGraph->Write(m_pPass, input, usage, subresource);
        }

        RGHandle Write(const RGHandle& input, u32 subresource = 0, RGBuilderFlag flag = RGBuilderFlag::None)
        {
            ngfx::GfxAccess::Flags state;

            switch (m_pPass->GetType())
            {
                case RenderPassType::Graphics:
                    if (flag == RGBuilderFlag::ShaderStagePS)
                    {
                        state = ngfx::GfxAccess::PixelShaderUAV;
                    }
                    else if (flag == RGBuilderFlag::ShaderStageNonPS)
                    {
                        state = ngfx::GfxAccess::VertexShaderUAV;
                    }
                    else
                    {
                        state = ngfx::GfxAccess::PixelShaderUAV | ngfx::GfxAccess::VertexShaderUAV;
                    }
                    break;
                case RenderPassType::Compute:
                case RenderPassType::AsyncCompute: state = ngfx::GfxAccess::ComputeUAV | ngfx::GfxAccess::ClearUAV; break;
                case RenderPassType::Copy: state = ngfx::GfxAccess::CopyDst; break;
                default: ASSERT(false); break;
            }
            return Write(input, state, subresource);
        }

        RGHandle WriteColor(u32 color_index, const RGHandle& input, u32 subresource, ngfx::GfxRenderPass::LoadOp load_op, float4 clear_color = float4(0.0f, 0.0f, 0.0f, 1.0f))
        {
            ASSERT(m_pPass->GetType() == RenderPassType::Graphics);
            return m_pGraph->WriteColor(m_pPass, color_index, input, subresource, load_op, clear_color);
        }

        RGHandle WriteDepth(const RGHandle& input, u32 subresource, ngfx::GfxRenderPass::LoadOp depth_load_op, float clear_depth = 0.0f)
        {
            ASSERT(m_pPass->GetType() == RenderPassType::Graphics);
            return m_pGraph->WriteDepth(m_pPass, input, subresource, depth_load_op, ngfx::GfxRenderPass::LoadDontCare, clear_depth, 0);
        }

        RGHandle WriteDepth(const RGHandle& input, u32 subresource, ngfx::GfxRenderPass::LoadOp depth_load_op, ngfx::GfxRenderPass::LoadOp stencil_load_op, float clear_depth = 0.0f, u32 clear_stencil = 0)
        {
            ASSERT(m_pPass->GetType() == RenderPassType::Graphics);
            return m_pGraph->WriteDepth(m_pPass, input, subresource, depth_load_op, stencil_load_op, clear_depth, clear_stencil);
        }

        RGHandle ReadDepth(const RGHandle& input, u32 subresource)
        {
            ASSERT(m_pPass->GetType() == RenderPassType::Graphics);
            return m_pGraph->ReadDepth(m_pPass, input, subresource);
        }

    private:
        RGBuilder(RGBuilder const&)            = delete;
        RGBuilder& operator=(RGBuilder const&) = delete;

    private:
        RenderGraph*         m_pGraph = nullptr;
        RenderGraphPassBase* m_pPass  = nullptr;
    };
} // namespace ncore
#endif

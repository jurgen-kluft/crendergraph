#ifndef __CRENDERGRAPH_RENDER_GRAPH_H__
#define __CRENDERGRAPH_RENDER_GRAPH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cdag/c_dag.h"
#include "crendergraph/render_graph_pass.h"
#include "crendergraph/render_graph_handle.h"
#include "crendergraph/render_graph_resource.h"
#include "crendergraph/render_graph_resource_allocator.h"
#include "callocator/c_allocator_linear.h"

namespace ncore
{
    class Renderer;
    class RenderGraphResourceNode;

    class RenderGraph
    {
        friend class RGBuilder;

    public:
        RenderGraph(Renderer* pRenderer);

        template <typename Data, typename Setup, typename Exec> RenderGraphPass<Data>& AddPass(cpstr_t name, RenderPassType type, const Setup& setup, const Exec& execute);

        void BeginEvent(cpstr_t name);
        void EndEvent();

        void Clear();
        void Compile();
        void Execute(Renderer* pRenderer, IGfxCommandList* pCommandList, IGfxCommandList* pComputeCommandList);

        void Present(const RGHandle& handle, GfxAccessFlags filnal_state);

        RGHandle Import(IGfxTexture* texture, GfxAccessFlags state);
        RGHandle Import(IGfxBuffer* buffer, GfxAccessFlags state);

        RGTexture* GetTexture(const RGHandle& handle);
        RGBuffer*  GetBuffer(const RGHandle& handle);

        const DirectedAcyclicGraph& GetDAG() const { return m_graph; }
        cpstr_t       Export(nstring::storage_t* strs);

    private:
        template <typename T, typename... ArgsT> T* Allocate(ArgsT&&... arguments);
        template <typename T, typename... ArgsT> T* AllocatePOD(ArgsT&&... arguments);

        template <typename Resource> RGHandle Create(const typename Resource::Desc& desc, cpstr_t name);

        RGHandle Read(RenderGraphPassBase* pass, const RGHandle& input, GfxAccessFlags usage, u32 subresource);
        RGHandle Write(RenderGraphPassBase* pass, const RGHandle& input, GfxAccessFlags usage, u32 subresource);

        RGHandle WriteColor(RenderGraphPassBase* pass, u32 color_index, const RGHandle& input, u32 subresource, GfxRenderPassLoadOp load_op, const float4& clear_color);
        RGHandle WriteDepth(RenderGraphPassBase* pass, const RGHandle& input, u32 subresource, GfxRenderPassLoadOp depth_load_op, GfxRenderPassLoadOp stencil_load_op, float clear_depth, u32 clear_stencil);
        RGHandle ReadDepth(RenderGraphPassBase* pass, const RGHandle& input, u32 subresource);

    private:
        linear_alloc_t*              m_allocator;
        RenderGraphResourceAllocator m_resourceAllocator;
        DirectedAcyclicGraph         m_graph;

        eastl::vector<cpstr_t> m_eventNames;

        eastl::unique_ptr<IGfxFence> m_pComputeQueueFence;
        u64                     m_nComputeQueueFenceValue = 0;

        eastl::unique_ptr<IGfxFence> m_pGraphicsQueueFence;
        u64                     m_nGraphicsQueueFenceValue = 0;

        eastl::vector<RenderGraphPassBase*>     m_passes;
        eastl::vector<RenderGraphResource*>     m_resources;
        eastl::vector<RenderGraphResourceNode*> m_resourceNodes;

        struct ObjFinalizer
        {
            void* obj;
            void (*finalizer)(void*);
        };
        eastl::vector<ObjFinalizer> m_objFinalizer;

        struct PresentTarget
        {
            RenderGraphResource* resource;
            GfxAccessFlags       state;
        };
        eastl::vector<PresentTarget> m_outputResources;
    };

    class RenderGraphEvent
    {
    public:
        RenderGraphEvent(RenderGraph* graph, cpstr_t name)
            : m_pRenderGraph(graph)
        {
            m_pRenderGraph->BeginEvent(name);
        }

        ~RenderGraphEvent() { m_pRenderGraph->EndEvent(); }

    private:
        RenderGraph* m_pRenderGraph;
    };

#define RENDER_GRAPH_EVENT(graph, event_name) RenderGraphEvent __graph_event__(graph, event_name)
} // namespace ncore

#include "crendergraph/render_graph.inl"

#endif

#ifndef __CRED_RENDER_GRAPH_PASS_H__
#define __CRED_RENDER_GRAPH_PASS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cdag/c_dag.h"

namespace ncore
{
    class Renderer;
    class RenderGraph;
    class RenderGraphResource;
    class RenderGraphEdgeColorAttchment;
    class RenderGraphEdgeDepthAttchment;

    class    IGfxCommandList;
    class    IGfxCommandList;
    class    IGfxFence;
    class    IGfxFence;

    enum class RenderPassType
    {
        Graphics,
        Compute,
        AsyncCompute,
        Copy,
    };

    struct RenderGraphAsyncResolveContext
    {
        eastl::vector<DAGNode*> computeQueuePasses;
        eastl::vector<DAGNode*> preGraphicsQueuePasses;
        eastl::vector<DAGNode*> postGraphicsQueuePasses;
        u64                 computeFence  = 0;
        u64                 graphicsFence = 0;
    };

    struct RenderGraphPassExecuteContext
    {
        Renderer*        renderer;
        IGfxCommandList* graphicsCommandList;
        IGfxCommandList* computeCommandList;
        IGfxFence*       computeQueueFence;
        IGfxFence*       graphicsQueueFence;

        u64 initialComputeFenceValue;
        u64 lastSignaledComputeValue;

        u64 initialGraphicsFenceValue;
        u64 lastSignaledGraphicsValue;
    };

    class RenderGraphPassBase : public DAGNode
    {
    public:
        RenderGraphPassBase(const nstring::str_t const* name, RenderPassType type, DirectedAcyclicGraph& graph);

        void ResolveBarriers(const DirectedAcyclicGraph& graph);
        void ResolveAsyncCompute(const DirectedAcyclicGraph& graph, RenderGraphAsyncResolveContext& context);
        void Execute(const RenderGraph& graph, RenderGraphPassExecuteContext& context);

        // virtual eastl::string GetGraphvizName() const override { return m_name.c_str(); }
        // virtual const char*   GetGraphvizColor() const override { return !IsCulled() ? "darkgoldenrod1" : "darkgoldenrod4"; }

        void BeginEvent(const nstring::str_t const* name) { m_eventNames.push_back(name); }
        void EndEvent() { m_nEndEventNum++; }

        RenderPassType GetType() const { return m_type; }
        DAGNode*      GetWaitGraphicsPassID() const { return m_waitGraphicsPass; }
        DAGNode*      GetSignalGraphicsPassID() const { return m_signalGraphicsPass; }

    private:
        void Begin(const RenderGraph& graph, IGfxCommandList* pCommandList);
        void End(IGfxCommandList* pCommandList);

        bool HasGfxRenderPass() const;

        virtual void ExecuteImpl(IGfxCommandList* pCommandList) = 0;

    protected:
        eastl::string  m_name;
        RenderPassType m_type;

        eastl::vector<eastl::string> m_eventNames;
        u32                          m_nEndEventNum = 0;

        struct ResourceBarrier
        {
            RenderGraphResource* resource;
            u32                  sub_resource;
            GfxAccessFlags       old_state;
            GfxAccessFlags       new_state;
        };
        eastl::vector<ResourceBarrier> m_resourceBarriers;

        struct AliasDiscardBarrier
        {
            IGfxResource*  resource;
            GfxAccessFlags acess_before;
            GfxAccessFlags acess_after;
        };
        eastl::vector<AliasDiscardBarrier> m_discardBarriers;

        RenderGraphEdgeColorAttchment* m_pColorRT[8] = {};
        RenderGraphEdgeDepthAttchment* m_pDepthRT    = nullptr;

        // only for async-compute pass
        DAGNode* m_waitGraphicsPass   = nullptr;
        DAGNode* m_signalGraphicsPass = nullptr;

        u64 m_signalValue = -1;
        u64 m_waitValue   = -1;
    };

    template <class T> class RenderGraphPass : public RenderGraphPassBase
    {
    public:
        RenderGraphPass(const nstring::str_t const* name, RenderPassType type, DirectedAcyclicGraph& graph, const eastl::function<void(const T&, IGfxCommandList*)>& execute)
            : RenderGraphPassBase(name, type, graph)
        {
            m_execute = execute;
        }

        T&       GetData() { return m_parameters; }
        T const* operator->() { return &GetData(); }

    private:
        void ExecuteImpl(IGfxCommandList* pCommandList) override { m_execute(m_parameters, pCommandList); }

    protected:
        T                                                 m_parameters;
        eastl::function<void(const T&, IGfxCommandList*)> m_execute;
    };
} // namespace ncore
#endif

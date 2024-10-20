#ifndef __CRENDERGRAPH_RENDER_GRAPH_INL__
#define __CRENDERGRAPH_RENDER_GRAPH_INL__

#include "crendergraph/render_graph.h"
#include "crendergraph/render_graph_builder.h"

namespace ncore
{
    class RenderGraphEdge : public DAGEdge
    {
    public:
        RenderGraphEdge(DirectedAcyclicGraph& graph, DAGNode* from, DAGNode* to, GfxAccessFlags usage, u32 subresource)
            : DAGEdge(graph, from, to)
        {
            m_usage       = usage;
            m_subresource = subresource;
        }

        GfxAccessFlags GetUsage() const { return m_usage; }
        u32            GetSubresource() const { return m_subresource; }

    private:
        GfxAccessFlags m_usage;
        u32            m_subresource;
    };

    class RenderGraphResourceNode : public DAGNode
    {
    public:
        RenderGraphResourceNode(DirectedAcyclicGraph& graph, RenderGraphResource* resource, u32 version)
            : DAGNode(graph)
            , m_graph(graph)
        {
            m_pResource = resource;
            m_version   = version;
        }

        RenderGraphResource* GetResource() const { return m_pResource; }
        u32                  GetVersion() const { return m_version; }

        virtual eastl::string GetGraphvizName() const override
        {
            eastl::string s = m_pResource->GetName();
            s.append("\nversion:");
            s.append(eastl::to_string(m_version));
            if (m_version > 0)
            {
                eastl::vector<DAGEdge*> incoming_edges;
                m_graph.GetIncomingEdges(this, incoming_edges);
                RE_ASSERT(incoming_edges.size() == 1);
                u32 subresource = ((RenderGraphEdge*)incoming_edges[0])->GetSubresource();
                s.append("\nsubresource:");
                s.append(eastl::to_string(subresource));
            }
            return s;
        }

        virtual const char* GetGraphvizColor() const override { return !IsCulled() ? "lightskyblue1" : "lightskyblue4"; }
        virtual const char* GetGraphvizShape() const override { return "ellipse"; }

    private:
        RenderGraphResource* m_pResource;
        u32                  m_version;

        DirectedAcyclicGraph& m_graph;
    };

    class RenderGraphEdgeColorAttchment : public RenderGraphEdge
    {
    public:
        RenderGraphEdgeColorAttchment(DirectedAcyclicGraph& graph, DAGNode* from, DAGNode* to, GfxAccessFlags usage, u32 subresource, u32 color_index, GfxRenderPassLoadOp load_op, const float4& clear_color)
            : RenderGraphEdge(graph, from, to, usage, subresource)
        {
            m_colorIndex = color_index;
            m_loadOp     = load_op;

            m_clearColor[0] = clear_color[0];
            m_clearColor[1] = clear_color[1];
            m_clearColor[2] = clear_color[2];
            m_clearColor[3] = clear_color[3];
        }
        u32                 GetColorIndex() const { return m_colorIndex; }
        GfxRenderPassLoadOp GetLoadOp() const { return m_loadOp; }
        const float*        GetClearColor() const { return m_clearColor; }

    private:
        u32                 m_colorIndex;
        GfxRenderPassLoadOp m_loadOp;
        float               m_clearColor[4] = {};
    };

    class RenderGraphEdgeDepthAttchment : public RenderGraphEdge
    {
    public:
        RenderGraphEdgeDepthAttchment(DirectedAcyclicGraph& graph, DAGNode* from, DAGNode* to, GfxAccessFlags usage, u32 subresource, GfxRenderPassLoadOp depth_load_op, GfxRenderPassLoadOp stencil_load_op, float clear_depth, u32 clear_stencil)
            : RenderGraphEdge(graph, from, to, usage, subresource)
        {
            m_depthLoadOp   = depth_load_op;
            m_stencilLoadOp = stencil_load_op;
            m_clearDepth    = clear_depth;
            m_clearStencil  = clear_stencil;
            m_bReadOnly     = (usage & GfxAccessDSVReadOnly) ? true : false;
        }

        GfxRenderPassLoadOp GetDepthLoadOp() const { return m_depthLoadOp; };
        GfxRenderPassLoadOp GetStencilLoadOp() const { return m_stencilLoadOp; };
        float               GetClearDepth() const { return m_clearDepth; }
        u32                 GetClearStencil() const { return m_clearStencil; };
        bool                IsReadOnly() const { return m_bReadOnly; }

    private:
        GfxRenderPassLoadOp m_depthLoadOp;
        GfxRenderPassLoadOp m_stencilLoadOp;
        float               m_clearDepth;
        u32                 m_clearStencil;
        bool                m_bReadOnly;
    };

    template <typename T> void ClassFinalizer(void* p) { ((T*)p)->~T(); }

    template <typename T, typename... ArgsT> inline T* RenderGraph::Allocate(ArgsT&&... arguments)
    {
        T* p = (T*)m_allocator.Alloc(sizeof(T));
        new (p) T(arguments...);

        ObjFinalizer finalizer;
        finalizer.obj       = p;
        finalizer.finalizer = &ClassFinalizer<T>;
        m_objFinalizer.push_back(finalizer);

        return p;
    }

    template <typename T, typename... ArgsT> inline T* RenderGraph::AllocatePOD(ArgsT&&... arguments)
    {
        T* p = (T*)m_allocator.Alloc(sizeof(T));
        new (p) T(arguments...);

        return p;
    }

    template <typename Data, typename Setup, typename Exec> inline RenderGraphPass<Data>& RenderGraph::AddPass(const eastl::string& name, RenderPassType type, const Setup& setup, const Exec& execute)
    {
        auto pass = Allocate<RenderGraphPass<Data>>(name, type, m_graph, execute);

        for (size_t i = 0; i < m_eventNames.size(); ++i)
        {
            pass->BeginEvent(m_eventNames[i]);
        }
        m_eventNames.clear();

        RGBuilder builder(this, pass);
        setup(pass->GetData(), builder);

        m_passes.push_back(pass);

        return *pass;
    }

    template <typename Resource> inline RGHandle RenderGraph::Create(const typename Resource::Desc& desc, const eastl::string& name)
    {
        auto resource = Allocate<Resource>(m_resourceAllocator, name, desc);
        auto node     = AllocatePOD<RenderGraphResourceNode>(m_graph, resource, 0);

        RGHandle handle;
        handle.index = (u16)m_resources.size();
        handle.node  = (u16)m_resourceNodes.size();

        m_resources.push_back(resource);
        m_resourceNodes.push_back(node);

        return handle;
    }

} // namespace ncore
#endif

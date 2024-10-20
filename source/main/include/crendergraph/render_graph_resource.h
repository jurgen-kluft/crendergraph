#ifndef __CRED_RENDER_GRAPH_RESOURCE_H__
#define __CRED_RENDER_GRAPH_RESOURCE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cdag/c_dag.h"
#include "callocator/c_allocator_string.h"

namespace ncore
{
    class RenderGraphEdge;
    class RenderGraphPassBase;
    class RenderGraphResourceAllocator;

    class IGfxBuffer;
    class IGfxTexture;
    class IGfxCommandList;

    class RenderGraphResource
    {
    public:
        RenderGraphResource(cpstr_t name) { m_name = name; }
        virtual ~RenderGraphResource() {}

        virtual void           Resolve(RenderGraphEdge* edge, RenderGraphPassBase* pass);
        virtual void           Realize()         = 0;
        virtual IGfxResource*  GetResource()     = 0;
        virtual GfxAccessFlags GetInitialState() = 0;

        cpstr_t  GetName() const { return m_name; }
        DAGNode* GetFirstPassID() const { return m_firstPass; }
        DAGNode* GetLastPassID() const { return m_lastPass; }

        bool IsUsed() const { return m_firstPass != UINT32_MAX; }
        bool IsImported() const { return m_bImported; }

        GfxAccessFlags GetFinalState() const { return m_lastState; }
        virtual void   SetFinalState(GfxAccessFlags state) { m_lastState = state; }

        bool IsOutput() const { return m_bOutput; }
        void SetOutput(bool value) { m_bOutput = value; }

        bool IsOverlapping() const { return !IsImported() && !IsOutput(); }

        virtual IGfxResource* GetAliasedPrevResource(GfxAccessFlags& lastUsedState)                                                            = 0;
        virtual void          Barrier(IGfxCommandList* pCommandList, u32 subresource, GfxAccessFlags acess_before, GfxAccessFlags acess_after) = 0;

    protected:
        cpstr_t m_name;

        DAGNode*       m_firstPass = UINT32_MAX;
        DAGNode*       m_lastPass  = 0;
        GfxAccessFlags m_lastState = GfxAccessDiscard;

        bool m_bImported = false;
        bool m_bOutput   = false;
    };

    class RGTexture : public RenderGraphResource
    {
    public:
        using Desc = GfxTextureDesc;

        RGTexture(RenderGraphResourceAllocator& allocator, const nstring::str_t const* name, const Desc& desc);
        RGTexture(RenderGraphResourceAllocator& allocator, IGfxTexture* texture, GfxAccessFlags state);
        ~RGTexture();

        IGfxTexture*    GetTexture() const { return m_pTexture; }
        IGfxDescriptor* GetSRV();
        IGfxDescriptor* GetUAV();
        IGfxDescriptor* GetUAV(u32 mip, u32 slice);

        virtual void           Resolve(RenderGraphEdge* edge, RenderGraphPassBase* pass) override;
        virtual void           Realize() override;
        virtual IGfxResource*  GetResource() override { return m_pTexture; }
        virtual GfxAccessFlags GetInitialState() override { return m_initialState; }
        virtual void           Barrier(IGfxCommandList* pCommandList, u32 subresource, GfxAccessFlags acess_before, GfxAccessFlags acess_after) override;
        virtual IGfxResource*  GetAliasedPrevResource(GfxAccessFlags& lastUsedState) override;

    private:
        Desc                          m_desc;
        IGfxTexture*                  m_pTexture     = nullptr;
        GfxAccessFlags                m_initialState = GfxAccessDiscard;
        RenderGraphResourceAllocator& m_allocator;
    };

    class RGBuffer : public RenderGraphResource
    {
    public:
        using Desc = GfxBufferDesc;

        RGBuffer(RenderGraphResourceAllocator& allocator, const nstring::str_t const* name, const Desc& desc);
        RGBuffer(RenderGraphResourceAllocator& allocator, IGfxBuffer* buffer, GfxAccessFlags state);
        ~RGBuffer();

        IGfxBuffer*     GetBuffer() const { return m_pBuffer; }
        IGfxDescriptor* GetSRV();
        IGfxDescriptor* GetUAV();

        virtual void           Resolve(RenderGraphEdge* edge, RenderGraphPassBase* pass) override;
        virtual void           Realize() override;
        virtual IGfxResource*  GetResource() override { return m_pBuffer; }
        virtual GfxAccessFlags GetInitialState() override { return m_initialState; }
        virtual void           Barrier(IGfxCommandList* pCommandList, u32 subresource, GfxAccessFlags acess_before, GfxAccessFlags acess_after) override;
        virtual IGfxResource*  GetAliasedPrevResource(GfxAccessFlags& lastUsedState) override;

    private:
        Desc                          m_desc;
        IGfxBuffer*                   m_pBuffer      = nullptr;
        GfxAccessFlags                m_initialState = GfxAccessDiscard;
        RenderGraphResourceAllocator& m_allocator;
    };
} // namespace ncore
#endif

#ifndef __CRENDERGRAPH_RENDER_GRAPH_RESOURCE_ALLOCATOR_H__
#define __CRENDERGRAPH_RENDER_GRAPH_RESOURCE_ALLOCATOR_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "callocator/c_allocator_string.h"
#include "cgfx/gfx_defines.h"

namespace ncore
{
    class IGfxDevice;
    class IGfxResource;
    class IGfxTexture;
    class IGfxBuffer;
    class IGfxDescriptor;
    class IGfxHeap;

    class RenderGraphResourceAllocator
    {
        struct LifetimeRange
        {
            u32 firstPass = UINT32_MAX;
            u32 lastPass  = 0;

            void Reset()
            {
                firstPass = UINT32_MAX;
                lastPass  = 0;
            }
            bool IsUsed() const { return firstPass != UINT32_MAX; }
            bool IsOverlapping(const LifetimeRange& other) const
            {
                if (IsUsed())
                {
                    return firstPass <= other.lastPass && lastPass >= other.firstPass;
                }
                else
                {
                    return false;
                }
            }
        };

        struct AliasedResource
        {
            IGfxResource*          resource;
            LifetimeRange          lifetime;
            u64                    lastUsedFrame = 0;
            ngfx::GfxAccess::Flags lastUsedState = ngfx::GfxAccess::Discard;
        };

        struct Heap
        {
            IGfxHeap* heap;
            // vector_t<AliasedResource> resources;
            s32              resources_size;
            AliasedResource* resources;

            bool IsOverlapping(const LifetimeRange& lifetime) const
            {
                for (size_t i = 0; i < resources_size; ++i)
                {
                    if (resources[i].lifetime.IsOverlapping(lifetime))
                    {
                        return true;
                    }
                }
                return false;
            }

            bool Contains(IGfxResource* resource) const
            {
                for (size_t i = 0; i < resources_size; ++i)
                {
                    if (resources[i].resource == resource)
                    {
                        return true;
                    }
                }
                return false;
            }
        };

        struct SRVDescriptor
        {
            IGfxResource*                   resource;
            IGfxDescriptor*                 descriptor;
            ngfx::GfxShaderResourceViewDesc desc;
        };

        struct UAVDescriptor
        {
            IGfxResource*                    resource;
            IGfxDescriptor*                  descriptor;
            ngfx::GfxUnorderedAccessViewDesc desc;
        };

    public:
        RenderGraphResourceAllocator(IGfxDevice* pDevice);
        ~RenderGraphResourceAllocator();

        void Reset();

        IGfxTexture* AllocateNonOverlappingTexture(const ngfx::GfxTextureDesc& desc, const nstring::str_t* name, ngfx::GfxAccess::Flags& initial_state);
        void         FreeNonOverlappingTexture(IGfxTexture* texture, ngfx::GfxAccess::Flags state);

        IGfxTexture* AllocateTexture(u32 firstPass, u32 lastPass, ngfx::GfxAccess::Flags lastState, const ngfx::GfxTextureDesc& desc, const nstring::str_t* name, ngfx::GfxAccess::Flags& initial_state);
        IGfxBuffer*  AllocateBuffer(u32 firstPass, u32 lastPass, ngfx::GfxAccess::Flags lastState, const ngfx::GfxBufferDesc& desc, const nstring::str_t* name, ngfx::GfxAccess::Flags& initial_state);
        void         Free(IGfxResource* resource, ngfx::GfxAccess::Flags state, bool set_state);

        IGfxResource* GetAliasedPrevResource(IGfxResource* resource, u32 firstPass, ngfx::GfxAccess::Flags& lastUsedState);

        IGfxDescriptor* GetDescriptor(IGfxResource* resource, const ngfx::GfxShaderResourceViewDesc& desc);
        IGfxDescriptor* GetDescriptor(IGfxResource* resource, const ngfx::GfxUnorderedAccessViewDesc& desc);

    private:
        void CheckHeapUsage(Heap& heap);
        void DeleteDescriptor(IGfxResource* resource);
        void AllocateHeap(u32 size);

    private:
        IGfxDevice* m_pDevice;

        // vector_t<Heap> m_allocatedHeaps;
        Heap* m_allocatedHeaps;
        s32   m_numAllocatedHeaps;
        s32   m_maxAllocatedHeaps;

        struct NonOverlappingTexture
        {
            IGfxTexture*           texture;
            ngfx::GfxAccess::Flags lastUsedState;
            u64                    lastUsedFrame;
        };
        // vector_t<NonOverlappingTexture> m_freeOverlappingTextures;
        NonOverlappingTexture* m_freeOverlappingTextures;
        s32                    m_numFreeOverlappingTextures;
        s32                    m_maxFreeOverlappingTextures;

        // vector_t<SRVDescriptor> m_allocatedSRVs;
        SRVDescriptor* m_allocatedSRVs;
        s32            m_numAllocatedSRVs;
        s32            m_maxAllocatedSRVs;
        // vector_t<UAVDescriptor> m_allocatedUAVs;
        UAVDescriptor* m_allocatedUAVs;
        s32            m_numAllocatedUAVs;
        s32            m_maxAllocatedUAVs;
    };

} // namespace ncore
#endif

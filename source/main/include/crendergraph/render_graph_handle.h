#ifndef __CRENDERGRAPH_RENDER_GRAPH_HANDLE_H__
#define __CRENDERGRAPH_RENDER_GRAPH_HANDLE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    struct RGHandle
    {
        u16 index = u16(-1);
        u16 node  = u16(-1);
        bool IsValid() const { return index != u16(-1) && node != u16(-1); }
    };
} // namespace ncore
#endif

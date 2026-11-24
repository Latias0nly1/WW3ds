#pragma once
#include <string>
#include <vector>
#include "ww_c3dplan.h"
namespace t2p 
{
    struct C3dMultiPlan 
    {
        bool ok = false;
        std::string why;
        bool hasClear = false;
        C3dPlan clear;
        C3dPlan first;
        C3dPlan second;
        C3dMultiPlan() = default;
        C3dMultiPlan(const C3dMultiPlan&) = delete;
        C3dMultiPlan& operator = (const C3dMultiPlan&) = delete;
        C3dMultiPlan(C3dMultiPlan 
&& ) = default;
        C3dMultiPlan& operator = (C3dMultiPlan 
&& ) = default;
        std::vector < uint8_t > clearVerts;
        std::vector < uint16_t > clearIdx;
        
    } ;
    C3dMultiPlan buildMultiPlan(const DrawPacket& P,
C3dShaderCache& shaders,
TextureCache& tex,
const GuestMem& mem,
TexEntry * scratchView,
bool diagnostics = true);
    
}

#pragma once
#include <string>
#include "ww_draw.h"
namespace t2p 
{
    struct MatErr 
    {
        double mean255 = 0, max255 = 0;
        
    } ;
    MatErr materialError(const TevMaterial& original,
const TevMaterial& degraded,
uint64_t seed,
int n);
    bool compileApprox(const GxDrawState&,
const Options&,
const PicaDraw& failed,
PicaDraw& out,
std::string& why);
    
}

#pragma once
#include <string>
#include <utility>
#include <vector>
#include "ww_draw.h"
namespace t2p 
{
    std::vector < std::pair < int, int >> crossingRegs(const TevMaterial& t,
int cut);
    struct SplitDraws 
    {
        GxDrawState first, second;
        bool depthOrdered = false;
        bool depthFar = true;
        bool exactFragments = true;
        std::string fragmentNote;
        std::string rematNote;
        
    } ;
    bool makeSplit(const GxDrawState& g,
int cut,
SplitDraws& out,
std::string& why);
    bool compileSplit(const GxDrawState& g,
const Options& opt,
PicaDraw& out,
std::string& why);
    
}

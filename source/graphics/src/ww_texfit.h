#pragma once
#include <cstdint>
#include <string>
namespace t2p 
{
    struct AxisPlan 
    {
        enum Fill 
        {
            Same, EdgePad, TileRepeat, TileMirror, Stretch 
        } ;
        int n = 0, D = 0;
        Fill fill = Same;
        uint8_t wrap = 0;
        bool exact = true;
        std::string note;
        
    } ;
    AxisPlan planAxis(int n,
int gxWrap);
    int axisSource(const AxisPlan& p,
int x,
int nl,
int Dl);
    float axisScale(const AxisPlan& p);
    
}

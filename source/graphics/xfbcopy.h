#pragma once
#include <cstdint>
#include <vector>
namespace hw6 
{
    inline void bpWrite(std::vector < uint8_t > & v,
uint8_t reg,
uint32_t val) 
    {
        const uint32_t w = (uint32_t(reg) << 24) | (val & 0xFFFFFF);
        v.push_back(0x61);
        v.push_back(uint8_t(w >> 24));
        v.push_back(uint8_t(w >> 16));
        v.push_back(uint8_t(w >> 8));
        v.push_back(uint8_t(w));
        
    }
    inline std::vector < uint8_t > xfbCopyBytes(const uint8_t rgba[4],
bool clear) 
    {
        std::vector < uint8_t > v;
        bpWrite(v,
0x40,
0x1F);
        bpWrite(v,
0x41,
0x18);
        bpWrite(v,
0x49,
0);
        bpWrite(v,
0x4A,
639u | (479u << 10));
        bpWrite(v,
0x4B,
0x00400000u >> 5);
        bpWrite(v,
0x4D,
640u * 2 / 32);
        bpWrite(v,
0x4F,
uint32_t(rgba[0]) | (uint32_t(rgba[3]) << 8));
        bpWrite(v,
0x50,
uint32_t(rgba[2]) | (uint32_t(rgba[1]) << 8));
        bpWrite(v,
0x51,
0xFFFFFF);
        bpWrite(v,
0x52,
(1u << 14) | (clear ? (1u << 11) : 0u));
        return v;
        
    }
    
}

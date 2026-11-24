#pragma once
#include <cstdint>
#include <string>
namespace t2p 
{
    enum : uint8_t 
    {
        PF_NEVER = 0, PF_ALWAYS = 1, PF_EQUAL = 2, PF_NOTEQUAL = 3, PF_LESS = 4, PF_LEQUAL = 5, PF_GREATER = 6, PF_GEQUAL = 7 
    } ;
    uint8_t picaFuncFromGx(int gxCompare);
    struct GxAlphaCompare 
    {
        uint8_t comp0 = 7, ref0 = 0, op = 1, comp1 = 7, ref1 = 0;
        
    } ;
    struct PicaAlphaTest 
    {
        bool enable = false;
        uint8_t func = PF_ALWAYS, ref = 0;
        bool exact = true;
        int mismatches = 0;
        std::string note;
        
    } ;
    bool gxAlphaPasses(const GxAlphaCompare&,
int alpha);
    PicaAlphaTest alphaFromGx(const GxAlphaCompare&);
    struct GxBlend 
    {
        uint8_t mode = 0, src = 1, dst = 0, logic = 3;
        
    } ;
    struct PicaBlend 
    {
        bool useLogicOp = false;
        uint8_t logicOp = 3;
        uint8_t eqRgb = 0, eqAlpha = 0;
        uint8_t srcRgb = 1, dstRgb = 0, srcAlpha = 1, dstAlpha = 0;
        bool blendsAnything = false;
        bool needsDstAlpha = false;
        bool exact = true;
        std::string note;
        
    } ;
    PicaBlend blendFromGx(const GxBlend&);
    struct GxZMode 
    {
        bool compare = true;
        uint8_t func = 3;
        bool update = true;
        
    } ;
    struct PicaDepth 
    {
        bool testEnable = true;
        uint8_t func = PF_LEQUAL;
        bool write = true;
        
    } ;
    PicaDepth depthFromGx(const GxZMode&);
    constexpr float kPicaDepthMax = 255.0f / 256.0f;
    constexpr float kPicaDepthClearFar = 1023.0f / 1024.0f;
    extern bool g_picaDepthFold;
    extern bool g_picaForceZDepth;
    extern bool g_picaDepthReverse;
    inline bool picaReverseZ() 
    {
        return g_picaDepthReverse 
&& g_picaForceZDepth 
&& g_picaDepthFold;
        
    }
    uint8_t picaMirrorDepthFunc(uint8_t f);
    inline uint32_t picaFrameClearDepth() 
    {
        return picaReverseZ() ? 0u : 0x00FFFFFFu;
        
    }
    inline float picaConstDepth(double g,
bool farClear) 
    {
        if (picaReverseZ()) return farClear ? 0.0f : float((1.0 - g) * double(kPicaDepthMax));
        return farClear ? kPicaDepthClearFar : float(g * double(kPicaDepthMax));
        
    }
    inline void picaFoldDepthRow(float row2[4],
const float row3[4],
float s,
float o) 
    {
        for (int j = 0; j < 4; j++) row2[j] = float( - (double(s) * row2[j] + double(o) * row3[j]));
        
    }
    struct PicaDepthMap 
    {
        bool zBuffer = true;
        float scale = 1.0f, offset = 1.0f;
        uint32_t clear = 0xFFFFFF;
        
    } ;
    PicaDepthMap depthMapZ();
    PicaDepthMap depthMapW(float rangeD);
    float picaDepthValue(const PicaDepthMap&,
float zOverW,
float wClip);
    uint32_t picaDepth24(const PicaDepthMap&,
float zOverW,
float wClip);
    struct GxFog 
    {
        uint8_t type = 0;
        float startZ = 0, endZ = 1, nearZ = 0.1f, farZ = 1;
        uint8_t r = 0, g = 0, b = 0;
        
    } ;
    float fogFactorGx(uint8_t type,
float t);
    struct PicaFog 
    {
        bool enable = false;
        uint32_t lut[128] = 
        {
            
        } ;
        uint32_t color = 0;
        float depthRange = 1;
        float depthScale = 0, depthOffset = 1;
        bool depthSaturates = false;
        float maxErr255 = 0;
        bool exact = true;
        std::string note;
        
    } ;
    PicaFog fogFromGx(const GxFog&,
float projFar,
bool shrink = true,
float forceRange = 0.0f);
    float fogDepthRange(const GxFog&,
float projFar,
bool shrink = true,
float forceRange = 0.0f);
    class FogCompileCache 
    {
        public:
        PicaFog get(const GxFog&,
float projFar,
bool shrink = true,
float forceRange = 0.0f);
        private:
        struct Entry 
        {
            uint32_t key[9] = 
            {
                
            } ;
            PicaFog value;
            uint64_t age = 0;
            bool valid = false;
            
        } ;
        Entry entries_[16];
        uint64_t age_ = 0;
        
    } ;
    float picaFogEval(const PicaFog&,
float depth01,
float lutScale = 2048.0f);
    std::string describe(const GxAlphaCompare&,
const PicaAlphaTest&);
    std::string describe(const GxBlend&,
const PicaBlend&);
    
}

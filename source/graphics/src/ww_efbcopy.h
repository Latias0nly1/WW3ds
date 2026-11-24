#pragma once
#include <string>
#include <vector>
#include "ww_c3dplan.h"
#include "ww_model.h"
namespace t2p 
{
    enum class CopyProg : uint8_t 
    {
        Unsupported, 
        Color, 
        ChanR, 
        ChanG, 
        ChanB, 
        ChanA, 
        ChanRA, 
        Luma, 
        LumaA, 
        
    } ;
    struct EfbCopyFormat 
    {
        bool known = false;
        const char * name = "";
        CopyProg prog = CopyProg::Unsupported;
        int pfmt = PTF_RGBA8;
        int sampleFmt = 6;
        std::string note;
        
    } ;
    EfbCopyFormat efbCopyFormat(const GxCopy& c);
    PicaProgram efbCopyProgram(CopyProg p,
bool hasAlpha);
    inline constexpr float kEfbCopyPrimary = 16.0f / 255.0f;
    struct EfbCopyOptions 
    {
        float frameFogRange = 0;
        
    } ;
    struct EfbCopyPlan 
    {
        bool ok = false;
        std::string why;
        bool toXfb = false;
        bool toTexture = false;
        RenderedTexture tex;
        int srcLogicalX0 = 0, srcLogicalY0 = 0, srcLogicalX1 = 0, srcLogicalY1 = 0;
        C3dPlan blit;
        bool clears = false;
        C3dPlan clear;
        bool clearsColor = false, clearsAlpha = false, clearsDepth = false;
        uint8_t clearColor[4] = 
        {
            
        } ;
        uint32_t clearZ = 0;
        std::vector < std::string > notes;
        EfbCopyPlan() = default;
        EfbCopyPlan(const EfbCopyPlan&) = delete;
        EfbCopyPlan& operator = (const EfbCopyPlan&) = delete;
        EfbCopyPlan(EfbCopyPlan 
&& ) = default;
        EfbCopyPlan& operator = (EfbCopyPlan 
&& ) = default;
        std::vector < uint8_t > blitVerts, clearVerts;
        std::vector < uint16_t > blitIdx, clearIdx;
        
    } ;
    EfbCopyPlan planEfbCopy(const GxCopy& c,
const GxState& at,
const FrameConfig& cfg,
C3dShaderCache& shaders,
TexEntry * mainView,
const EfbCopyOptions& opt = 
    {
        
    }
    );
    std::string efbBlitShaderSource();
    bool planTexturedMesh(C3dPlan& L,
C3dShaderCache& shaders,
TexEntry * tex,
int maxLevel,
const std::vector < uint8_t > & verts,
const std::vector < uint16_t > & idx,
int w,
int h,
std::string& why);
    std::string efbClearShaderSource();
    uint32_t efbCopyGuestBytes(int sampleFmt,
int gxW,
int gxH);
    
}

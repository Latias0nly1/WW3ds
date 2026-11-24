#include "ww_uniform_storage.h"
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "ww_draw.h"
#include "ww_gxfifo.h"
namespace t2p 
{
    struct GxTexBinding 
    {
        bool valid = false;
        uint32_t addr = 0;
        int width = 0, height = 0;
        int fmt = 0;
        uint8_t wrapS = 0, wrapT = 0;
        bool magLinear = false, minLinear = false;
        uint8_t mipMode = 0;
        float lodBias = 0, minLod = 0, maxLod = 0;
        bool edgeLod = false, biasClamp = false;
        uint8_t aniso = 0;
        uint32_t tlutOffset = 0;
        int tlutFmt = 0;
        int mipLevels() const;
        
    } ;
    struct GxViewport 
    {
        float x = 0, y = 0, width = 0, height = 0, nearZ = 0, farZ = 1;
        
    } ;
    struct GxScissor 
    {
        int x = 0, y = 0, width = 0, height = 0;
        
    } ;
    struct GxProjection 
    {
        bool ortho = false;
        float p[6] = 
        {
            
        } ;
        float nearZ = 0, farZ = 0;
        float m[4][4] = 
        {
            
        } ;
        
    } ;
    struct GxCopy 
    {
        bool toXfb = false;
        bool clear = false;
        bool halfScale = false;
        bool depth = false;
        bool intensity = false;
        int fmt = 0;
        int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
        uint32_t dstAddr = 0;
        uint32_t dstStrideBytes = 0;
        uint8_t clearColor[4] = 
        {
            
        } ;
        uint32_t clearZ = 0;
        
    } ;
    struct GxTlutLoad 
    {
        uint32_t srcAddr = 0;
        uint32_t tmemOffset = 0;
        uint32_t entries = 0;
        
    } ;
    struct GxState 
    {
        GxDrawState draw;
        #if defined(__3DS__) || defined(T2P_TEST_SHARED_UNIFORMS)
std::shared_ptr < const VtxUniforms > uniformBlock;
        std::shared_ptr < const CompactUniforms > sectionUniforms;
        #else
VtxUniforms uniforms;
        #endif
VtxUniformView uniformView()const
        {
            #if defined(__3DS__) || defined(T2P_TEST_SHARED_UNIFORMS)
if(uniformBlock)return uniformViewOf( * uniformBlock);
            if(sectionUniforms)return sectionUniforms->view();
            #endif
return uniformViewOf(vertexUniforms());
            
        }
        const VtxUniforms& vertexUniforms() const 
        {
            #if defined(__3DS__) || defined(T2P_TEST_SHARED_UNIFORMS)
static const VtxUniforms identity;
            return uniformBlock ? * uniformBlock : sectionUniforms ? sectionUniforms->get() : identity;
            #else
return uniforms;
            #endif
}
        Rgba kcolor[4];
        Rgba tevReg[4];
        GxTexBinding tex[8];
        GxViewport viewport;
        GxScissor scissor;
        GxProjection proj;
        uint8_t cullMode = 2;
        bool colorUpdate = true, alphaUpdate = true, dither = false;
        bool dstAlphaEnable = false;
        uint8_t dstAlpha = 0;
        uint8_t pixelFmt = 0;
        uint8_t zFmt16 = 0;
        bool zCompBeforeTex = false;
        int numTexGens = 0, numChans = 0, numTevStages = 0, numIndStages = 0;
        int curPosMtx = 0;
        int uniformPosSel = 0;
        std::vector < std::string > notes;
        
    } ;
    constexpr int kScanPosMtx = - 1000;
    GxState decodeState(const GxRegs&,
const GxDraw * draw = nullptr,
bool notes = true,
int knownMaxPm = kScanPosMtx);
    void setStateDecodeMemo(bool on);
    void decodeStateInto(GxState& S,
const GxRegs&,
const GxDraw * draw = nullptr,
bool notes = true,
int knownMaxPm = kScanPosMtx);
    void decodeStateIncremental(GxState& S,
const GxRegs&,
const GxDraw * draw,
bool notes,
int knownMaxPm,
uint32_t dirty,
const GxState * prev);
    void decodeStateTemplate(GxState&,
const GxRegs&,
const GxDraw * ,
int knownMaxPm,
uint32_t dirty,
const GxState * previous,
const GxState& material);
    GxCopy decodeCopy(const GxRegs&);
    GxTlutLoad decodeTlutLoad(const GxRegs&);
    
}

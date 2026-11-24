#pragma once
#include <string>
#include <vector>
#include "ww_gxstate.h"
#include "ww_texconv.h"
#include "ww_texfit.h"
namespace t2p 
{
    struct PicaSamplerDesc 
    {
        uint8_t wrapS = 0, wrapT = 0;
        bool magLinear = false, minLinear = false;
        uint8_t mipFilter = 0;
        int minLevel = 0, maxLevel = 0;
        float lodBias = 0;
        
    } ;
    struct MappedTexture 
    {
        bool ok = false;
        std::string error;
        int fmt = PTF_RGBA8;
        int width = 0, height = 0;
        int levels = 1;
        std::vector < uint8_t > data;
        size_t encodedBytes = 0;
        bool externalOutput = false;
        std::vector < size_t > levelOffset;
        float scaleS = 1, scaleT = 1;
        PicaSamplerDesc sampler;
        bool exact = true;
        double maxErr255 = 0, meanErr255 = 0;
        std::vector < std::string > notes;
        std::vector < std::string > info;
        
    } ;
    struct TexMapOptions 
    {
        bool needAlpha = true;
        TexConvOptions conv;
        
    } ;
    PicaSamplerDesc samplerFor(const GxTexBinding& b,
int levels);
    size_t gxImageBytes(const GxTexBinding& b);
    MappedTexture mapTexture(const GxTexBinding& b,
const uint8_t * image,
size_t imageSize,
const GxTlut * tlut,
const TexMapOptions& opt = 
    {
        
    }
    );
    
}

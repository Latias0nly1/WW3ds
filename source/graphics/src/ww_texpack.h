#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include "ww_gxstate.h"
namespace t2p 
{
    struct TexPackKey 
    {
        uint64_t imageHash = 0;
        uint64_t paletteKey = 0;
        uint16_t width = 0, height = 0;
        uint8_t fmt = 0, wrapS = 0, wrapT = 0, levels = 0, needAlpha = 0, tlutFmt = 0, pad[2] = 
        {
            0, 0
        } ;
        uint32_t reserved = 0;
        bool operator == (const TexPackKey& o) const;
        uint64_t hash() const;
        
    } ;
    static_assert(sizeof(TexPackKey) == 32,
"TexPackKey layout");
    struct TexPackHeader 
    {
        char magic[4] = 
        {
            'W', 'W', 'T', 'P'
        } ;
        uint32_t version = 2;
        uint32_t count = 0;
        uint32_t optionsSig = 0;
        
    } ;
    struct TexPackIndex 
    {
        uint64_t keyHash;
        uint32_t dataOffset;
        uint32_t meta;
        
    } ;
    struct TexPackRecord 
    {
        TexPackKey key;
        uint8_t pfmt = 0, plevels = 0, exact = 0, ok = 0;
        uint16_t pw = 0, ph = 0;
        float scaleS = 1, scaleT = 1;
        uint32_t encodedBytes = 0;
        uint32_t dataOffset = 0, dataBytes = 0;
        uint32_t levelOffset[11] = 
        {
            
        } ;
        
    } ;
    bool texPackKey(const GxTexBinding& b,
const uint8_t * image,
size_t imageBytes,
const uint8_t * tlutRaw,
size_t tlutAvail,
bool needAlpha,
TexPackKey& k);
    uint32_t texPackOptionsSig(bool allowLossy,
bool tryEtc1,
bool fastLossyError);
    uint32_t texPackMeta(const TexPackRecord& r);
    bool texPackLayout(uint32_t meta,
const GxTexBinding& b,
TexPackRecord& rec);
    class TexPackReader 
    {
        public:
        using ReadFn = std::function < bool(uint64_t,
size_t,
void * ) > ;
        bool open(ReadFn read,
uint32_t optionsSig);
        bool ok() const 
        {
            return !index_.empty();
            
        }
        size_t count() const 
        {
            return index_.size();
            
        }
        bool find(const TexPackKey& k,
const GxTexBinding& b,
TexPackRecord& rec) const;
        bool readData(const TexPackRecord& rec,
void * dst) const 
        {
            return read_(rec.dataOffset,
rec.dataBytes,
dst);
            
        }
        size_t indexBytes() const 
        {
            return index_.capacity() * sizeof(TexPackIndex);
            
        }
        private:
        ReadFn read_;
        std::vector < TexPackIndex > index_;
        
    } ;
    
}
static_assert(sizeof(t2p::TexPackRecord) == 104 
&& sizeof(t2p::TexPackIndex) == 16 
&& sizeof(t2p::TexPackHeader) == 16,
"texpack layout (PC writer = 3DS reader)");

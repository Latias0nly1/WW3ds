#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "ww_translate.h"
namespace t2p 
{
    class ByteWriter 
    {
        public:
        std::vector < uint8_t > buf;
        void u8(uint32_t v) 
        {
            buf.push_back(uint8_t(v));
            
        }
        void u16(uint32_t v) 
        {
            u8(v);
            u8(v >> 8);
            
        }
        void u32(uint32_t v) 
        {
            u16(v);
            u16(v >> 16);
            
        }
        void u64(uint64_t v) 
        {
            u32(uint32_t(v));
            u32(uint32_t(v >> 32));
            
        }
        void i32(int32_t v) 
        {
            u32(uint32_t(v));
            
        }
        void f32(float v);
        void f64(double v);
        void str(const std::string& s) 
        {
            u32(uint32_t(s.size()));
            buf.insert(buf.end(),
s.begin(),
s.end());
            
        }
        
    } ;
    class ByteReader 
    {
        public:
        ByteReader(const uint8_t * p,
size_t n) : p_(p), n_(n) 
        {
            
        }
        bool ok() const 
        {
            return ok_;
            
        }
        size_t remaining() const 
        {
            return n_ - at_;
            
        }
        void fail() 
        {
            ok_ = false;
            at_ = n_;
            
        }
        uint8_t u8() 
        {
            if (at_ >= n_) 
            {
                fail();
                return 0;
                
            }
            return p_[at_++];
            
        }
        uint16_t u16() 
        {
            const uint32_t a = u8();
            return uint16_t(a | u8() << 8);
            
        }
        uint32_t u32() 
        {
            const uint32_t a = u16();
            return a | uint32_t(u16()) << 16;
            
        }
        uint64_t u64() 
        {
            const uint64_t a = u32();
            return a | uint64_t(u32()) << 32;
            
        }
        int32_t i32() 
        {
            return int32_t(u32());
            
        }
        float f32();
        double f64();
        std::string str(size_t maxLen = 1 << 20);
        uint32_t count(uint32_t max,
size_t minBytes);
        private:
        const uint8_t * p_;
        size_t n_, at_ = 0;
        bool ok_ = true;
        
    } ;
    void putTevMaterial(ByteWriter&,
const TevMaterial&,
bool withTexcoord);
    bool getTevMaterial(ByteReader&,
TevMaterial&);
    void putTranslation(ByteWriter&,
const Translation&);
    bool getTranslation(ByteReader&,
Translation&);
    std::string translateKey(const TevMaterial&,
const Options&);
    std::string approxKey(const TevMaterial&);
    struct ApproxEntry 
    {
        TevMaterial material;
        bool relaxed = false;
        std::string note;
        
    } ;
    class MaterialBundle 
    {
        public:
        const Translation * findTranslation(const std::string& key) const;
        const ApproxEntry * findApprox(const std::string& key) const;
        void addTranslation(const std::string& key,
const Translation&);
        void addApprox(const std::string& key,
const ApproxEntry&);
        size_t translations() const 
        {
            return tr_.size();
            
        }
        size_t approximations() const 
        {
            return ap_.size();
            
        }
        std::vector < uint8_t > serialize(uint64_t fingerprint) const;
        bool load(const uint8_t * data,
size_t size,
uint64_t fingerprint,
std::string * err = nullptr);
        private:
        std::unordered_map < std::string, Translation > tr_;
        std::unordered_map < std::string, ApproxEntry > ap_;
        
    } ;
    void setBundleLookup(const MaterialBundle * );
    void setBundleRecorder(MaterialBundle * );
    const MaterialBundle * bundleLookup();
    MaterialBundle * bundleRecorder();
    struct BundleStats 
    {
        uint64_t hits = 0, misses = 0;
        double missSeconds = 0;
        uint64_t approxHits = 0;
        
    } ;
    BundleStats bundleStats();
    void resetBundleStats();
    void bundleCountHit();
    void bundleCountMiss(double seconds);
    void bundleCountApproxHit();
    uint64_t sourceFingerprint();
    
}

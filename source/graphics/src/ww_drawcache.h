#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>
#include "ww_draw.h"
namespace t2p 
{
    uint64_t hashDrawKey(const GxDrawState&,
const Options&);
    void replayDrawKey(const GxDrawState&,
uint64_t out[2]);
    bool sameDrawKey(const GxDrawState& a,
const Options& oa,
const GxDrawState& b,
const Options& ob);
    bool sameDrawKeyIgnoringPoseCount(const GxDrawState&,
const GxDrawState&);
    struct DrawCacheStats 
    {
        uint64_t lookups = 0, hits = 0, misses = 0, evictions = 0;
        size_t entries = 0;
        
    } ;
    class DrawCompileCache 
    {
        public:
        explicit DrawCompileCache(size_t maxEntries = 4096,
uint64_t hashMask = ~0ull) : max_(maxEntries ? maxEntries : 1), mask_(hashMask) 
        {
            
        }
        std::shared_ptr < const PicaDraw > get(const GxDrawState& g,
const Options& opt);
        void clear();
        void setRecentKeyFastPath(bool enabled) 
        {
            recentKeyFastPath_ = enabled;
            
        }
        DrawCacheStats stats() const 
        {
            DrawCacheStats s = st_;
            s.entries = lru_.size();
            return s;
            
        }
        private:
        struct Entry 
        {
            uint64_t hash = 0;
            GxDrawState key;
            Options opt;
            std::shared_ptr < const PicaDraw > value;
            
        } ;
        bool recentKeyFastPath_ = true;
        size_t max_;
        uint64_t mask_;
        std::list < Entry > lru_;
        std::unordered_multimap < uint64_t, std::list < Entry > ::iterator > byHash_;
        DrawCacheStats st_;
        
    } ;
    
}

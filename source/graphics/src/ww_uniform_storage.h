#pragma once
#include "ww_vtx.h"
#include "ww_uniform_pieces.h"
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <new>
namespace t2p 
{
    struct FixedBlockPool 
    {
        struct Node 
        {
            Node * next;
            
        } ;
        explicit FixedBlockPool(unsigned c,
size_t sz = 0) : cap(c), size(sz) 
        {
            
        }
        const unsigned cap;
        std::mutex mu;
        Node * head = nullptr;
        unsigned count = 0;
        size_t size;
        unsigned long long hits = 0, misses = 0;
        void * take(size_t n) 
        {
            
            {
                std::lock_guard < std::mutex > g(mu);
                if (!size) size = n;
                if (n == size 
&& head) 
                {
                    Node * x = head;
                    head = x->next;
                    --count;
                    ++hits;
                    return x;
                    
                }
                if (n == size) ++misses;
                
            }
            return ::operator new(n);
            
        }
        void give(void * p,
size_t n) 
        {
            
            {
                std::lock_guard < std::mutex > g(mu);
                if (n == size 
&& count < cap) 
                {
                    Node * x = static_cast < Node * > (p);
                    x->next = head;
                    head = x;
                    ++count;
                    return;
                    
                }
                
            }
            ::operator delete(p);
            
        }
        void trim() 
        {
            trimTo(0);
            
        }
        void trimTo(unsigned keep) 
        {
            Node * list = nullptr;
            
            {
                std::lock_guard < std::mutex > g(mu);
                while (count > keep 
&& head) 
                {
                    Node * x = head;
                    head = x->next;
                    x->next = list;
                    list = x;
                    --count;
                    
                }
                
            }
            while (list) 
            {
                Node * x = list;
                list = x->next;
                ::operator delete(x);
                
            }
            
        }
        
    } ;
    struct UniformBlockPool 
    {
        static FixedBlockPool& get() 
        {
            static FixedBlockPool * p = new FixedBlockPool(256);
            return * p;
            
        }
        
    } ;
    template < class T, class Pool > struct PoolAlloc 
    {
        using value_type = T;
        PoolAlloc() = default;
        template < class U > PoolAlloc(const PoolAlloc < U,
Pool > &) 
        {
            
        }
        template < class U > struct rebind 
        {
            using other = PoolAlloc < U, Pool > ;
            
        } ;
        T * allocate(size_t n) 
        {
            return static_cast < T * > (Pool::get().take(n * sizeof(T)));
            
        }
        void deallocate(T * p,
size_t n) 
        {
            Pool::get().give(p,
n * sizeof(T));
            
        }
        template < class U > bool operator == (const PoolAlloc < U,
Pool > &) const 
        {
            return true;
            
        }
        template < class U > bool operator != (const PoolAlloc < U,
Pool > &) const 
        {
            return false;
            
        }
        
    } ;
    template < class T > using UniformBlockAlloc = PoolAlloc < T, UniformBlockPool > ;
    struct CompactBlockPool 
    {
        static FixedBlockPool& get() 
        {
            static FixedBlockPool * p = new FixedBlockPool(512);
            return * p;
            
        }
        
    } ;
    template < class... A > std::shared_ptr < VtxUniforms > makeUniformBlock(A 
&& ... a) 
    {
        return std::allocate_shared < VtxUniforms > (UniformBlockAlloc < VtxUniforms > (),
std::forward < A > (a)...);
        
    }
    struct UniformNonPose 
    {
        float proj[4][4], matColor[4], ambColor[4];
        VtxLight light[8];
        float texMtx[10][3][4], texScale[3][2];
        explicit UniformNonPose(const VtxUniforms& u)
        {
            std::memcpy(proj,
u.proj,
sizeof proj);
            std::memcpy(matColor,
u.matColor,
sizeof matColor);
            std::memcpy(ambColor,
u.ambColor,
sizeof ambColor);
            std::memcpy(light,
u.light,
sizeof light);
            std::memcpy(texMtx,
u.texMtx,
sizeof texMtx);
            std::memcpy(texScale,
u.texScale,
sizeof texScale);
            
        }
        
    } ;
    static_assert(sizeof(UniformNonPose) == 1112,
"float rows without padding");
    inline bool sameUsedNonPose(const VtxSetup& setup,
const VtxShader& shader,
const UniformNonPose& a,
const UniformNonPose& b)
    {
        if(std::memcmp(a.proj,
b.proj,
sizeof a.proj) || std::memcmp(a.matColor,
b.matColor,
sizeof a.matColor) || 
        std::memcmp(a.ambColor,
b.ambColor,
sizeof a.ambColor) || std::memcmp(a.texScale,
b.texScale,
sizeof a.texScale))return false;
        for (int i=0;i<8;++i)if(shader.lightSlot[i] >= 0 
&& std::memcmp(&a.light[i],
&b.light[i],
sizeof a.light[i]))return false;
        for (size_t i=0;i<setup.texgen.size()
&&i<3;++i)if(shader.texMtxSlot[i] >= 0)
        {
            const int k = (setup.texgen[i].mtx - 30) / 3;
            if(k < 0 || k >= 10 || std::memcmp(a.texMtx[k],
b.texMtx[k],
sizeof a.texMtx[k]))return false;
            
        }
        return true;
        
    }
    struct CompactUniforms 
    {
        const float ( * pos)[3][4] = nullptr;
        const float ( * nrm)[3][4] = nullptr;
        std::shared_ptr < const UniformNonPose > shared;
        std::shared_ptr < const UniformPieces > pieces;
        int npose = 10;
        mutable std::atomic < const VtxUniforms * > published
        {
            nullptr
        } ;
        ~CompactUniforms()
        {
            delete published.load(std::memory_order_relaxed);
            
        }
        static void poseRows(float ( * pos)[3][4],
float ( * nrm)[3][4],
const uint32_t * xf,
int selected,
int n)
        {
            const int from = selected?selected:0;
            std::memcpy(pos,
xf + from * 12,
size_t(n) * sizeof pos[0]);
            const uint32_t * xw = xf + 0x400 + from * 9;
            uint32_t * nw = reinterpret_cast < uint32_t * > (&nrm[0][0][0]);
            for (int row=0;row<3*n;++row,
nw+=4,
xw+=3)
            {
                nw[0] = xw[0];
                nw[1] = xw[1];
                nw[2] = xw[2];
                nw[3] = 0;
                
            }
            
        }
        CompactUniforms(int n,
std::shared_ptr < const UniformNonPose > p):shared(std::move(p)), npose(n)
        {
            
        }
        CompactUniforms(int n,
std::shared_ptr < const UniformPieces > p):pieces(std::move(p)), npose(n)
        {
            
        }
        bool samePose(const uint32_t * xf,
int selected,
int n)const
        {
            if(npose < n)return false;
            const int from = selected?selected:0;
            if(std::memcmp(pos,
xf + from * 12,
size_t(n) * sizeof pos[0]))return false;
            const uint32_t * xw = xf + 0x400 + from * 9;
            const uint32_t * nw = reinterpret_cast < const uint32_t * > (&nrm[0][0][0]);
            for (int row=0;row<3*n;++row,
nw+=4,
xw+=3)if(nw[0] != xw[0] || nw[1] != xw[1] || nw[2] != xw[2])return false;
            return true;
            
        }
        bool matches(const VtxUniforms& u,
int n)const
        {
            if(npose < n || std::memcmp(pos,
u.pos,
size_t(n) * sizeof pos[0]) || std::memcmp(nrm,
u.nrm,
size_t(n) * sizeof nrm[0]))return false;
            if(pieces)
            {
                const auto v = view();
                return !std::memcmp(v.proj,
u.proj,
sizeof u.proj) 
&& !std::memcmp(v.matColor,
u.matColor,
sizeof u.matColor) 
&& !std::memcmp(v.ambColor,
u.ambColor,
sizeof u.ambColor) 
&& !std::memcmp(v.light,
u.light,
sizeof u.light) 
&& !std::memcmp(v.texMtx,
u.texMtx,
sizeof u.texMtx) 
&& !std::memcmp(v.texScale,
u.texScale,
sizeof u.texScale);
                
            }
            const UniformNonPose p(u);
            return !std::memcmp(shared.get(),
&p,
sizeof p);
            
        }
        VtxUniformView view()const
        {
            if(pieces)return pieces->view(pos,
nrm,
npose);
            return 
            {
                shared->proj, pos, nrm, shared->matColor, shared->ambColor, shared->light, shared->texMtx, shared->texScale, npose
            } ;
            
        }
        const VtxUniforms& get()const
        {
            if(const auto * p = published.load(std::memory_order_acquire))return * p;
            auto u = std::make_unique < VtxUniforms > ();
            std::memcpy(u->pos,
pos,
size_t(npose) * sizeof pos[0]);
            std::memcpy(u->nrm,
nrm,
size_t(npose) * sizeof nrm[0]);
            const auto v = view();
            std::memcpy(u->proj,
v.proj,
sizeof u->proj);
            std::memcpy(u->matColor,
v.matColor,
sizeof u->matColor);
            std::memcpy(u->ambColor,
v.ambColor,
sizeof u->ambColor);
            std::memcpy(u->light,
v.light,
sizeof u->light);
            std::memcpy(u->texMtx,
v.texMtx,
sizeof u->texMtx);
            std::memcpy(u->texScale,
v.texScale,
sizeof u->texScale);
            const VtxUniforms * expected = nullptr;
            if(published.compare_exchange_strong(expected,
u.get(),
std::memory_order_acq_rel,
std::memory_order_acquire))return * u.release();
            return * expected;
            
        }
        
    } ;
    template < int N > struct CompactUniformRows final : CompactUniforms 
    {
        float storage[2][N][3][4];
        CompactUniformRows(const uint32_t * xf,
int selected,
int n,
std::shared_ptr < const UniformNonPose > p):CompactUniforms(n,
std::move(p))
        {
            pos = storage[0];
            nrm = storage[1];
            poseRows(storage[0],
storage[1],
xf,
selected,
n);
            
        }
        CompactUniformRows(const uint32_t * xf,
int selected,
int n,
std::shared_ptr < const UniformPieces > p):CompactUniforms(n,
std::move(p))
        {
            pos = storage[0];
            nrm = storage[1];
            poseRows(storage[0],
storage[1],
xf,
selected,
n);
            
        }
        CompactUniformRows(const VtxUniforms& u,
int n,
std::shared_ptr < const UniformNonPose > p):CompactUniforms(n,
std::move(p))
        {
            pos = storage[0];
            nrm = storage[1];
            std::memcpy(storage[0],
u.pos,
size_t(n) * sizeof storage[0][0]);
            std::memcpy(storage[1],
u.nrm,
size_t(n) * sizeof storage[1][0]);
            
        }
        
    } ;
    extern bool g_compactPoseStorage;
    template < class... Args > 
    std::shared_ptr < const CompactUniforms > allocateCompactUniforms(int n,
Args 
&& ... args)
    {
        if(n < 1 || n > 10)throw std::bad_alloc();
        const int storage = g_compactPoseStorage?n:10;
         #define CPOSE_CASE(N) case N:return std::allocate_shared<const CompactUniformRows<N>>(PoolAlloc<CompactUniformRows<N>,CompactBlockPool>(),std::forward<Args>(args)...)
switch(storage)
        {
            CPOSE_CASE(1);
            CPOSE_CASE(2);
            CPOSE_CASE(3);
            CPOSE_CASE(4);
            CPOSE_CASE(5);
            CPOSE_CASE(6);
            CPOSE_CASE(7);
            CPOSE_CASE(8);
            CPOSE_CASE(9);
            CPOSE_CASE(10);
            
        }
         #undef CPOSE_CASE
throw std::bad_alloc();
        
    }
    inline std::shared_ptr < const CompactUniforms > makeCompactUniforms(const VtxUniforms& u,
int n,
std::shared_ptr < const UniformNonPose > p)
    {
        return allocateCompactUniforms(n,
u,
n,
std::move(p));
        
    }
    inline std::shared_ptr < const CompactUniforms > makeCompactUniforms(const uint32_t * xf,
int selected,
int n,
std::shared_ptr < const UniformNonPose > p)
    {
        return allocateCompactUniforms(n,
xf,
selected,
n,
std::move(p));
        
    }
    inline std::shared_ptr < const CompactUniforms > makeCompactPieceUniforms(const uint32_t * xf,
int selected,
int n,
std::shared_ptr < const UniformPieces > p)
    {
        return allocateCompactUniforms(n,
xf,
selected,
n,
std::move(p));
        
    }
    extern bool g_uniformSections;
    std::shared_ptr < const CompactUniforms > compactUniforms(const VtxUniforms&,
int npose = 10);
    
}

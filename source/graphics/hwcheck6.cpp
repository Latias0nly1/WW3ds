#include <3ds.h>
#include <citro3d.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "ww_backend3ds.h"
#include "ww_bundle.h"
#include "ww_c3dplan.h"
#include "ww_efbcopy.h"
#include "ww_frame.h"
#include "ww_gxfifo.h"
#include "ww_gxstate.h"
#include "hw6_mem.h"
#include "ww_multiplan.h"
#include "ww_submit.h"
#include "ww_texcache.h"
#include "hwcheck6_data.h"
u32 __stacksize__ = 0x100000;
static const int TW = 240, TH = 400;
static FILE * g_out = nullptr;
static t2p::MaterialBundle g_bundle;
static bool g_haveBundle = false;
static FILE * g_frames = nullptr;
static FILE * g_copies = nullptr;
static double ms(u64 ticks) 
{
    return double(ticks) * 1000.0 / double(SYSCLOCK_ARM11);
    
}
struct Runner : t2p::GxSink 
{
    t2p::Backend3ds * be = nullptr;
    t2p::TextureCache * cache = nullptr;
    t2p::C3dShaderCache * shaders = nullptr;
    hw6::FlatMem * mem = nullptr;
    t2p::FrameConfig cfg;
    t2p::TexEntry * view = nullptr;
    C3D_RenderTarget * tgt = nullptr;
    C3D_RenderTarget * scratch = nullptr;
    t2p::TexEntry * scratchView = nullptr;
    int twoPass = 0;
    const char * scene = "";
    int idx = 0, copyIdx = 0;
    int planned = 0, drawn = 0, refused = 0;
    double tBuild = 0, tPlan = 0, tDraw = 0, tSearchTotal = 0;
    struct Ev 
    {
        bool isCopy = false;
        t2p::GxState st;
        t2p::GxDraw d;
        t2p::GxCopy c;
        
    } ;
    std::vector < Ev > evs;
    struct Dump 
    {
        std::string name;
        uint32_t addr;
        int allocW, allocH, portW, portH, pfmt, gxFmt;
        
    } ;
    std::vector < Dump > dumps;
    void onDraw(const t2p::GxRegs& r,
const t2p::GxDraw& d) override 
    {
        Ev x;
        x.d = d;
        x.st = t2p::decodeState(r,
&d);
        evs.push_back(std::move(x));
        
    }
    void onCopy(const t2p::GxRegs& r) override 
    {
        Ev x;
        x.isCopy = true;
        x.c = t2p::decodeCopy(r);
        x.st = t2p::decodeState(r,
nullptr);
        evs.push_back(std::move(x));
        
    }
    void flush() 
    {
        std::vector < t2p::GxDrawState > states;
        for (const Ev& x : evs) if (!x.isCopy) states.push_back(x.st.draw);
        t2p::Options opt;
        opt.approxEffort = 0;
        opt.frameFogRange = t2p::frameFogRange(states.data(),
states.size());
        if (opt.frameFogRange > 0) std::fprintf(g_out,
"F\t%s\tfog frame: W-buffer range D = %g\n",
scene,
double(opt.frameFogRange));
        for (const Ev& x : evs) 
        {
            if (x.isCopy) copy(x,
opt);
            else one(x.st,
x.d,
opt);
            
        }
        evs.clear();
        
    }
    void copy(const Ev& x,
const t2p::Options& opt) 
    {
        t2p::EfbCopyOptions co;
        co.frameFogRange = opt.frameFogRange;
        const t2p::EfbCopyFormat F = t2p::efbCopyFormat(x.c);
        const u64 t0 = svcGetSystemTick();
        t2p::EfbCopyPlan plan = t2p::planEfbCopy(x.c,
x.st,
cfg,
* shaders,
view,
co);
        const u64 t1 = svcGetSystemTick();
        bool done = false;
        std::string why = plan.why;
        if (plan.ok) 
        {
            done = be->copy(plan,
* cache,
tgt);
            if (!done) why = be->lastError();
            
        }
        const u64 t2 = svcGetSystemTick();
        std::fprintf(g_out,
"C\t%s\t%d\tfmt=%s\tok=%d\ttoTexture=%d\tclears=%d\tdone=%d\tsrc=%d,%d,%d,%d\tdst=%08x\tgx=%dx%d\talloc=%dx%d\tport=%dx%d\tpfmt=%d\tplan %.2f ms draw %.2f ms\t%s\n",
        scene,
copyIdx,
F.name,
int(plan.ok),
int(plan.toTexture),
int(plan.clears),
int(done),
x.c.srcX,
x.c.srcY,
x.c.srcW,
x.c.srcH,
unsigned(x.c.dstAddr),
        plan.tex.gxW,
plan.tex.gxH,
plan.tex.allocW,
plan.tex.allocH,
plan.tex.portW,
plan.tex.portH,
plan.tex.pfmt,
ms(t1 - t0),
ms(t2 - t1),
why.c_str());
        for (const std::string& n : plan.notes) std::fprintf(g_out,
"N\t%s\tcopy%d\tplan\t%s\n",
scene,
copyIdx,
n.c_str());
        if (done 
&& plan.toTexture) 
        {
            char nm[32];
            std::snprintf(nm,
sizeof nm,
"%s#%d",
scene,
copyIdx);
            dumps.push_back(
            {
                nm,
plan.tex.addr,
plan.tex.allocW,
plan.tex.allocH,
plan.tex.portW,
plan.tex.portH,
plan.tex.pfmt,
plan.tex.gxFmt
            }
            );
            
        }
        copyIdx++;
        std::fflush(g_out);
        
    }
    void writeDumps() 
    {
        for (const Dump& d : dumps) 
        {
            const t2p::TexEntry * e = cache->renderedAt(d.addr);
            const void * data = nullptr;
            uint32_t bytes = 0;
            const bool have = e 
&& be->textureImage( * e,
&data,
&bytes);
            if (!have) 
            {
                std::fprintf(g_out,
"C\t%s\tdump FAILED (texture gone)\n",
d.name.c_str());
                bytes = 0;
                
            }
            char name[32] = 
            {
                
            } ;
            std::snprintf(name,
sizeof name,
"%s",
d.name.c_str());
            const uint16_t hdr[4] = 
            {
                uint16_t(d.allocW), uint16_t(d.allocH), uint16_t(d.portW), uint16_t(d.portH)
            } ;
            const uint32_t tail[3] = 
            {
                uint32_t(d.pfmt), uint32_t(d.gxFmt), bytes
            } ;
            if (g_copies) 
            {
                std::fwrite(name,
1,
sizeof name,
g_copies);
                std::fwrite(hdr,
2,
4,
g_copies);
                std::fwrite(tail,
4,
3,
g_copies);
                if (bytes) std::fwrite(data,
1,
bytes,
g_copies);
                std::fflush(g_copies);
                
            }
            
        }
        dumps.clear();
        
    }
    void one(const t2p::GxState& st,
const t2p::GxDraw& d,
const t2p::Options& opt) 
    {
        double tSearch = 0;
        bool sameAsSearch = true;
        if (g_haveBundle) 
        {
            t2p::setBundleLookup(nullptr);
            const u64 a = svcGetSystemTick();
            const t2p::DrawPacket p0 = t2p::buildDraw(st,
d,
opt,
cfg);
            tSearch = ms(svcGetSystemTick() - a);
            t2p::setBundleLookup(&g_bundle);
            const t2p::DrawPacket p1 = t2p::buildDraw(st,
d,
opt,
cfg);
            sameAsSearch = p0.ok == p1.ok 
&& p0.notes == p1.notes 
&& p0.vertexCount == p1.vertexCount 
&& p0.indices == p1.indices;
            tSearchTotal += tSearch;
            
        }
        const u64 t0 = svcGetSystemTick();
        t2p::DrawPacket pk = t2p::buildDraw(st,
d,
opt,
cfg);
        const u64 t1 = svcGetSystemTick();
        if (g_haveBundle) 
        {
            std::fprintf(g_out,
"M\t%s\t%d\tsearch %.2f ms bundle %.2f ms same=%d\n",
scene,
idx,
tSearch,
ms(t1 - t0),
int(sameAsSearch));
            
        }
        t2p::C3dPlan plan;
        t2p::C3dMultiPlan multi;
        std::vector < std::string > keepPlanNotes;
        const bool two = pk.ok 
&& pk.second;
        if (two) 
        {
            multi = t2p::buildMultiPlan(pk,
* shaders,
* cache,
* mem,
scratchView);
            for (const std::string& n : multi.first.notes) keepPlanNotes.push_back(n);
            for (const std::string& n : multi.second.notes) keepPlanNotes.push_back("second pass: " + n);
            
        } else 
        {
            plan = t2p::buildPlan(pk,
* shaders,
* cache,
* mem);
            keepPlanNotes = plan.notes;
            
        }
        const bool planOk = two ? multi.ok : plan.ok;
        const u64 t2 = svcGetSystemTick();
        bool drew = false;
        std::string why = pk.ok ? (two ? multi.why : plan.why) : std::string("packet refused");
        if (pk.ok 
&& planOk) 
        {
            planned++;
            drew = two ? be->drawTwoPass(multi,
scratch,
tgt) : be->draw(plan);
            if (!drew) why = be->lastError();
            else if (two) twoPass++;
            
        }
        const u64 t3 = svcGetSystemTick();
        tBuild += ms(t1 - t0);
        tPlan += ms(t2 - t1);
        tDraw += ms(t3 - t2);
        if (drew) drawn++;
        else refused++;
        std::fprintf(g_out,
"D\t%s\t%d\tpacket=%d\tplan=%d\tdrawn=%d\tverts=%u\ttris=%u\ttwopass=%d\t%s\n",
scene,
idx,
int(pk.ok),
int(planOk),
int(drew),
unsigned(pk.vertexCount),
        unsigned(pk.indices.size() / 3),
int(two),
why.c_str());
        for (const std::string& n : pk.notes) std::fprintf(g_out,
"N\t%s\t%d\tpacket\t%s\n",
scene,
idx,
n.c_str());
        for (const std::string& n : keepPlanNotes) std::fprintf(g_out,
"N\t%s\t%d\tplan\t%s\n",
scene,
idx,
n.c_str());
        idx++;
        std::fflush(g_out);
        
    }
    void onError(const std::string& e) override 
    {
        std::fprintf(g_out,
"E\t%s\t%s\n",
scene,
e.c_str());
        
    }
    
} ;
static int bail(const char * msg) 
{
    std::printf("\nERROR: %s\nPress START to exit.\n",
msg);
    if (g_out) 
    {
        std::fprintf(g_out,
"# ERROR %s\n",
msg);
        std::fclose(g_out);
        g_out = nullptr;
        
    }
    while (aptMainLoop()) 
    {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        gfxFlushBuffers();
        gfxScreenSwapBuffers(GFX_BOTTOM,
false);
        gspWaitForVBlank();
        
    }
    C3D_Fini();
    gfxExit();
    return 1;
    
}
int main() 
{
    gfxInitDefault();
    C3D_Init(0x100000);
    consoleInit(GFX_BOTTOM,
nullptr);
    std::printf("hwcheck6: %d scenes\n",
kHw6NumScenes - 1);
    g_out = std::fopen("sdmc:/hwcheck6_result.txt",
"w");
    if (!g_out) return bail("cannot write sdmc:/hwcheck6_result.txt");
    std::fprintf(g_out,
"# hwcheck6 v1 scenes=%d target=%dx%d\n",
kHw6NumScenes - 1,
TW,
TH);
    if (kHw6Bundle_size) 
    {
        std::string err;
        g_haveBundle = g_bundle.load(kHw6Bundle,
kHw6Bundle_size,
t2p::sourceFingerprint(),
&err);
        std::fprintf(g_out,
"M\tload\t%s\t%zu translations %zu approximations %u bytes\t%s\n",
g_haveBundle ? "ok" : "REFUSED",
g_bundle.translations(),
g_bundle.approximations(),
kHw6Bundle_size,
err.c_str());
        
    }
    g_frames = std::fopen("sdmc:/hwcheck6_frames.bin",
"wb");
    if (!g_frames) return bail("cannot write sdmc:/hwcheck6_frames.bin");
    g_copies = std::fopen("sdmc:/hwcheck6_copies.bin",
"wb");
    if (!g_copies) return bail("cannot write sdmc:/hwcheck6_copies.bin");
    C3D_RenderTarget * tgt = C3D_RenderTargetCreate(TW,
TH,
GPU_RB_RGBA8,
GPU_RB_DEPTH24_STENCIL8);
    if (!tgt) return bail("render target create failed");
    C3D_RenderTargetSetOutput(tgt,
GFX_TOP,
GFX_LEFT,
GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
    hw6::FlatMem mem;
    if (!mem.load(kHw6Mem,
kHw6Mem_size)) return bail("guest memory image is corrupt");
    t2p::Backend3ds be;
    be.setLog([](const std::string& s) 
    {
        if (g_out) 
        {
            std::fprintf(g_out,
"B\t%s\n",
s.c_str()); 
        }
        
    }
    );
    if (!be.init()) return bail("backend init failed (linear memory?)");
    t2p::TextureCache cache;
    cache.setEvictCallback([&](const t2p::TexEntry& e) 
    {
        be.release(e); 
    }
    );
    t2p::C3dShaderCache shaders;
    t2p::TexEntry mainView;
    if (!be.setMainView(mainView,
tgt)) return bail("main view texture failed");
    C3D_RenderTarget * scratch = C3D_RenderTargetCreate(TW,
TH,
GPU_RB_RGBA8,
GPU_RB_DEPTH24_STENCIL8);
    if (!scratch) return bail("scratch target create failed");
    t2p::TexEntry scratchView;
    if (!be.setMainView(scratchView,
scratch)) return bail("scratch view texture failed");
    Runner run;
    run.view = &mainView;
    run.tgt = tgt;
    run.scratch = scratch;
    run.scratchView = &scratchView;
    run.be = &be;
    run.cache = &cache;
    run.shaders = &shaders;
    run.mem = &mem;
    run.cfg.efbW = 640;
    run.cfg.efbH = 480;
    run.cfg.targetW = 400;
    run.cfg.targetH = 240;
    run.cfg.fit = t2p::FitMode::Pillarbox43;
    run.cfg.rotate = true;
    run.cfg.resolve();
    t2p::GxFifo fifo(&run);
    run.scene = "init";
    fifo.process(kHw6Stream + kHw6Scenes[0].off,
kHw6Scenes[0].len);
    int pending = - 1;
    for (int s = 1; s <= kHw6NumScenes; s++) 
    {
        C3D_FrameBegin(0);
        if (pending >= 0) 
        {
            const Hw6Scene& P = kHw6Scenes[pending];
            u8 * buf = reinterpret_cast < u8 * > (tgt->frameBuf.colorBuf);
            GSPGPU_InvalidateDataCache(buf,
TW * TH * 4);
            char name[32] = 
            {
                
            } ;
            std::snprintf(name,
sizeof name,
"%s",
P.name);
            const bool wrote = g_frames 
&& std::fwrite(name,
1,
sizeof name,
g_frames) == sizeof name 
&& std::fwrite(buf,
1,
size_t(TW) * TH * 4,
g_frames) == size_t(TW) * TH * 4;
            if (g_frames) std::fflush(g_frames);
            run.writeDumps();
            std::fprintf(g_out,
"S\t%s\t%s\tdraws=%d drawn=%d refused=%d twopass=%d\tbuild %.2f ms plan %.2f ms draw %.2f ms\n",
P.name,
wrote ? "written" : "WRITE FAILED",
run.idx,
run.drawn,
run.refused,
run.twoPass,
            run.tBuild,
run.tPlan,
run.tDraw);
            std::fflush(g_out);
            pending = - 1;
            
        }
        if (s == kHw6NumScenes) 
        {
            C3D_FrameEnd(0);
            break;
            
        }
        const Hw6Scene& S = kHw6Scenes[s];
        run.scene = S.name;
        run.idx = 0;
        run.copyIdx = 0;
        run.planned = run.drawn = run.refused = run.twoPass = 0;
        run.tBuild = run.tPlan = run.tDraw = 0;
        be.beginFrame();
        cache.beginFrame(mem);
        const u32 clear = (u32(S.clear[0]) << 24) | (u32(S.clear[1]) << 16) | (u32(S.clear[2]) << 8) | S.clear[3];
        C3D_RenderTargetClear(tgt,
C3D_CLEAR_ALL,
clear,
0x00FFFFFF);
        be.beginTarget(tgt);
        fifo.process(kHw6Stream + S.off,
S.len);
        run.flush();
        C3D_FrameEnd(0);
        pending = s;
        std::printf("scene %d/%d %s\n",
s,
kHw6NumScenes - 1,
S.name);
        gfxFlushBuffers();
        gfxScreenSwapBuffers(GFX_BOTTOM,
false);
        
    }
    const t2p::Backend3ds::Stats& bs = be.stats();
    const t2p::TexCacheStats cs = cache.stats();
    std::fprintf(g_out,
"# backend draws=%u refused=%u noArena=%u noCmdBuf=%u twoPass=%u splits=%u shaders=%u textures=%u textureBytes=%llu arenaPeak=%llu stateSkips=%u\n",
unsigned(bs.draws),
    unsigned(bs.refused),
unsigned(bs.noArena),
unsigned(bs.noCmdBuf),
unsigned(bs.twoPass),
unsigned(bs.splits),
unsigned(bs.shadersLoaded),
unsigned(bs.texturesUploaded),
    static_cast < unsigned long long > (bs.textureBytes),
static_cast < unsigned long long > (bs.arenaPeak),
unsigned(bs.stateSkips));
    std::fprintf(g_out,
"# texcache lookups=%llu hits=%llu misses=%llu conversions=%llu entries=%zu bytes=%zu\n",
static_cast < unsigned long long > (cs.lookups),
    static_cast < unsigned long long > (cs.hits),
static_cast < unsigned long long > (cs.misses),
static_cast < unsigned long long > (cs.conversions),
cs.entries,
cs.bytes);
    std::fprintf(g_out,
"# done\n");
    std::fclose(g_out);
    g_out = nullptr;
    std::fclose(g_frames);
    g_frames = nullptr;
    std::fclose(g_copies);
    g_copies = nullptr;
    shaders.forEach([&](t2p::C3dShader& sh) 
    {
        be.releaseShader(sh); 
    }
    );
    std::printf("\nDone. Results: sdmc:/hwcheck6_result.txt, hwcheck6_frames.bin, hwcheck6_copies.bin\nSTART = exit.\n");
    while (aptMainLoop()) 
    {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        gfxFlushBuffers();
        gfxScreenSwapBuffers(GFX_BOTTOM,
false);
        gspWaitForVBlank();
        
    }
    be.shutdown();
    C3D_Fini();
    gfxExit();
    return 0;
    
}

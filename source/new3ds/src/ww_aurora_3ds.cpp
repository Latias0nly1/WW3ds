#include <atomic>
#include <aurora/aurora.h>
#include <aurora/time.hpp>
#include <aurora/event.h>
#include <aurora/gfx.h>
#include <dolphin/gx/GXAurora.h>
#include "gx/command_processor.hpp"
#include "gx/fifo.hpp"
#include <cstdlib>
#include <cstring>
#include <string>
#include "ww_n3ds.h"
#include "ww_typed_gx_api_3ds.h"
extern "C" u64 svcGetSystemTick(void);
extern "C" void n3dsNextThreadStack(size_t bytes);
void JKRPcBeginHostAlloc();
void JKRPcEndHostAlloc();
namespace 
{
    struct HostAlloc 
    {
        HostAlloc() 
        {
            JKRPcBeginHostAlloc();
            
        }
        ~HostAlloc() 
        {
            JKRPcEndHostAlloc();
            
        }
        
    } ;
    
}
namespace aurora 
{
    AuroraConfig g_config;
    char g_gameName[5];
    
}
namespace 
{
    AuroraEvent sEvents[2];
    AuroraStats sStats
    {
        
    } ;
    bool sQuit = false;
    const char * dupOr(const char * s,
const char * fallback) 
    {
        return strdup(s != nullptr ? s : fallback);
        
    }
    
}
namespace aurora::gx::fifo 
{
    ProcessResult process(const uint8_t * data,
uint32_t size) noexcept 
    {
        int drawDone = 0;
        HostAlloc host;
        const uint32_t used = n3dsGpuProcess(data,
size,
&drawDone);
        return ProcessResult
        {
            used, drawDone != 0
        } ;
        
    }
    void clear_draw_cache() noexcept 
    {
        
    }
    void set_fast_paths(bool) noexcept 
    {
        
    }
    
}
namespace aurora::gfx::render_worker 
{
    bool is_worker_thread() noexcept 
    {
        return false;
        
    }
    
}
namespace aurora::gfx::depth_peek 
{
    extern "C" int n3dsReadClearDepth(uint16_t x,
uint16_t y);
    bool read_latest(uint16_t x,
uint16_t y,
uint32_t& z) noexcept 
    {
        if (n3dsReadClearDepth(x,
y)) 
        {
            z = 0xFFFFFF;
            return true;
            
        }
        z = 0;
        return false;
        
    }
    
}
namespace aurora::gx 
{
    void set_viewport_policy(AuroraViewportPolicy) noexcept 
    {
        
    }
    
}
namespace aurora::window 
{
    AuroraWindowSize get_window_size() 
    {
        return AuroraWindowSize
        {
            400, 240, 640, 480, 400, 240, 1.0f
        } ;
        
    }
    void set_title(const char * ) 
    {
        
    }
    void set_fullscreen(bool) 
    {
        
    }
    bool get_fullscreen() 
    {
        return true;
        
    }
    void set_window_size(uint32_t,
uint32_t) 
    {
        
    }
    void set_window_position(uint32_t,
uint32_t) 
    {
        
    }
    void center_window() 
    {
        
    }
    void request_frame_buffer_resize() 
    {
        
    }
    void set_frame_buffer_scale(float) 
    {
        
    }
    void set_frame_buffer_aspect_fit(bool) 
    {
        
    }
    void set_frame_buffer_fit_aspect(float) 
    {
        
    }
    void set_background_input(bool) 
    {
        
    }
    
}
extern "C" 
{
    AuroraInfo aurora_initialize(int,
char * [],
const AuroraConfig * config) 
    {
        aurora::g_config = * config;
        aurora::g_config.appName = dupOr(config->appName,
"ww3ds");
        aurora::g_config.userPath = dupOr(config->userPath,
N3DS_ROOT "/user");
        aurora::g_config.cachePath = dupOr(config->cachePath,
N3DS_ROOT "/cache");
        aurora::g_config.resourcesPath = dupOr(config->resourcesPath,
N3DS_ROOT);
        if (aurora::g_config.msaa == 0) aurora::g_config.msaa = 1;
        bool gpuOk;
        
        {
            HostAlloc host;
            gpuOk = n3dsGpuInit() != 0;
            
        }
        if (!gpuOk) 
        {
            n3dsLog("[3ds] GPU set-up failed\n");
            
        }
        n3dsNextThreadStack(512u << 10);
        aurora::gx::fifo::init();
        if (aurora::gx::fifo::processing_mode() != aurora::gx::fifo::ProcessingMode::Thread) n3dsNextThreadStack(0);
        if (aurora::gx::fifo::processing_mode() == aurora::gx::fifo::ProcessingMode::Thread) aurora::gx::fifo::set_async_end_frame(true);
        AuroraInfo info
        {
            
        } ;
        info.backend = BACKEND_NULL;
        info.userPath = aurora::g_config.userPath;
        info.cachePath = aurora::g_config.cachePath;
        info.window = reinterpret_cast < SDL_Window * > (1);
        info.windowSize = aurora::window::get_window_size();
        return info;
        
    }
    void aurora_shutdown() 
    {
        aurora::gx::fifo::shutdown();
        n3dsGpuShutdown();
        
    }
    const AuroraEvent * aurora_update() 
    {
        int n = 0;
        if (!sQuit 
&& !n3dsAptLoop()) sQuit = true;
        if (sQuit) sEvents[n++].type = AURORA_EXIT;
        sEvents[n].type = AURORA_NONE;
        return sEvents;
        
    }
    static bool sFrameOpen = false;
    bool aurora_begin_frame() 
    {
        if (sFrameOpen) return true;
        sFrameOpen = true;
        aurora::gx::fifo::begin_frame();
        n3dsTypedBeginFrame();
        return true;
        
    }
    extern "C" unsigned long long svcGetSystemTick(void);
    extern "C" void n3dsDrainCallersLog(void);
    static inline uint64_t n3dsTick() 
    {
        return svcGetSystemTick();
        
    }
    extern "C" 
    {
        extern unsigned long long g_n3dsDrainTicks, g_n3dsSyncTicks;
        extern unsigned g_n3dsDrainCalls, g_n3dsSyncCalls;
        
    }
    extern "C" int n3dsPaintSkipped(void);
    extern "C" int n3dsEarlyReleaseOn(void);
    extern "C" void svcSleepThread(long long ns);
    static uint32_t sEndSeq = 0;
    static std::atomic < uint32_t > sReleased
    {
        0
    } ;
    static bool sReleasedThisJob = false;
    extern "C" void n3dsPaintEnded(void);
    extern "C" void n3dsPaintHandedOff(void);
    extern "C" void n3dsEndFrameRelease(void) 
    {
        if (sReleasedThisJob) return;
        sReleasedThisJob = true;
        sReleased.fetch_add(1,
std::memory_order_release);
        
    }
    void aurora_end_frame() 
    {
        if (n3dsPaintSkipped()) return;
        sFrameOpen = false;
        n3dsPaintEnded();
        n3dsTypedEndFrame();
        if (aurora::gx::fifo::async_end_frame()) 
        {
            static const bool profWait = std::getenv("N3DS_PROF_WAIT") != nullptr;
            const uint64_t w0 = profWait ? n3dsTick() : 0;
            if (n3dsEarlyReleaseOn()) 
            {
                const uint32_t seq = ++sEndSeq;
                aurora::gx::fifo::end_frame_async([] 
                {
                    HostAlloc host; sReleasedThisJob = false; n3dsGpuEndFrame(); if (!sReleasedThisJob) n3dsEndFrameRelease(); n3dsPaintHandedOff(); 
                }
                );
                while (sReleased.load(std::memory_order_acquire) < seq) svcSleepThread(50000);
                
            } else 
            {
                aurora::gx::fifo::end_frame_async([] 
                {
                    HostAlloc host; n3dsGpuEndFrame(); n3dsPaintHandedOff(); 
                }
                );
                aurora::gx::fifo::sync();
                
            }
            if (profWait) 
            {
                static uint64_t waitTicks = 0, last = 0;
                static unsigned frames = 0;
                const uint64_t w1 = n3dsTick();
                waitTicks += w1 - w0;
                if (++frames % 60 == 0) 
                {
                    if (last) n3dsLog("[game-wait] 60 frames: period %.2f ms, blocked at frame end %.2f ms, in fifo drain %.2f ms (%.1f calls), in fifo sync %.2f ms (%.1f calls) per frame\n",
double(w1 - last) / 268111.856 / 60,
double(waitTicks) / 268111.856 / 60,
double(g_n3dsDrainTicks) / 268111.856 / 60,
g_n3dsDrainCalls / 60.0,
double(g_n3dsSyncTicks) / 268111.856 / 60,
g_n3dsSyncCalls / 60.0);
                    n3dsDrainCallersLog();
                    last = w1;
                    waitTicks = 0;
                    g_n3dsDrainTicks = g_n3dsSyncTicks = 0;
                    g_n3dsDrainCalls = g_n3dsSyncCalls = 0;
                    
                }
                
            }
            return;
            
        }
        aurora::gx::fifo::drain();
        aurora::gx::fifo::end_frame();
        
        {
            HostAlloc host;
            n3dsGpuEndFrame();
            
        }
        n3dsPaintHandedOff();
        
    }
    void aurora_set_async_end_frame(bool) 
    {
        
    }
    bool aurora_get_async_end_frame() 
    {
        return false;
        
    }
    void aurora_frame_sync() 
    {
        aurora::gx::fifo::sync();
        n3dsGpuSync();
        
    }
    void aurora_fifo_profile_enable(bool) 
    {
        
    }
    void aurora_fifo_profile_get(uint64_t * out,
size_t count) 
    {
        for (size_t i = 0; i < count; i++) out[i] = 0;
        
    }
    void aurora_set_fifo_fast_paths(bool) 
    {
        
    }
    void aurora_set_log_level(AuroraLogLevel level) 
    {
        aurora::g_config.logLevel = level;
        
    }
    void aurora_set_pause_on_focus_lost(bool) 
    {
        
    }
    void aurora_set_background_input(bool) 
    {
        
    }
    void aurora_set_resampler(AuroraSampler) 
    {
        
    }
    AuroraBackend aurora_get_backend() 
    {
        return BACKEND_NULL;
        
    }
    const AuroraBackend * aurora_get_available_backends(size_t * count) 
    {
        if (count) * count = 0;
        return nullptr;
        
    }
    const AuroraStats * aurora_get_stats() 
    {
        return &sStats;
        
    }
    float aurora_get_fps() 
    {
        return 0.f;
        
    }
    void aurora_enable_vsync(bool) 
    {
        
    }
    void aurora_save_unbuilt_pipelines(void) 
    {
        
    }
    
}
extern "C" void n3dsSettingsClockPause(int paused)
{
    static float previous = 1.0f;
    static bool active = false;
    if(paused 
&& !active)
    {
        aurora::gx::fifo::sync();
        previous = aurora::time::scale();
        aurora::time::set_scale(0);
        active = true;
        
    }
    else if(!paused 
&& active)
    {
        aurora::time::set_scale(previous);
        active = false;
        
    }
    
}

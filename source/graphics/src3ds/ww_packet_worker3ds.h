#pragma once
#include <functional>
#include <memory>
#include <string>
#include "ww_submit.h"
#include "ww_drawcache.h"
namespace t2p 
{
    extern int g_packetWorkerCore;
    class PacketWorker3ds 
    {
        public:
        PacketWorker3ds();
        ~PacketWorker3ds();
        PacketWorker3ds(const PacketWorker3ds&) = delete;
        PacketWorker3ds& operator = (const PacketWorker3ds&) = delete;
        bool start(const std::function < void(const std::string&) > & log);
        void stop();
        bool active() const;
        static constexpr size_t BatchSize = 16;
        struct Input 
        {
            const GxState * st = nullptr;
            const GxDraw * draw = nullptr;
            
        } ;
        void submitBatch(const Input * ,
size_t,
const Options&,
const FrameConfig&,
DrawCompileCache * ,
bool notes);
        void submit(const GxState&,
const GxDraw&,
const Options&,
const FrameConfig&,
DrawCompileCache * ,
bool notes);
        const DrawPacket& acquire(bool timed);
        void release();
        void resetStats();
        uint64_t waitTicks() const;
        private:
        struct Impl;
        std::unique_ptr < Impl > impl_;
        
    } ;
    
}

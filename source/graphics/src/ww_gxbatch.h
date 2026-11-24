#pragma once
#include <cstdint>
#include "ww_gxfifo.h"
namespace t2p 
{
    size_t compactDrawStorage(GxDraw&);
    constexpr uint32_t kBatchMaxVertices = 8192;
    bool canAppend(const GxDraw& into,
const GxDraw& d,
uint32_t maxVertices = kBatchMaxVertices);
    bool canAppendHeader(const GxDraw& into,
const GxDraw& header,
uint32_t maxVertices = kBatchMaxVertices);
    int maxPosMtxFrom(const GxDraw&,
uint32_t first);
    void appendDraw(GxDraw& into,
const GxDraw& d);
    int maxPosMtx(const GxDraw&);
    
}

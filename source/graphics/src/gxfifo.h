#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "gxdecodecache.h"
#include "decoded_vector.h"
#include "gxstateprogram.h"
#include "prepared_shape.h"
#include <array>

#ifdef __3DS__
extern "C" {
#include <3ds/types.h>
#include <3ds/svc.h>
}
extern bool g_t2pProfOn;
extern uint32_t g_t2pProfPeriod;
extern thread_local bool g_t2pProfNow;
inline bool t2pProfEnabled() { return g_t2pProfNow; }
inline uint64_t t2pProfTicks() { return g_t2pProfNow ? svcGetSystemTick() : 0; }
#else
inline bool t2pProfEnabled() { return false; }
inline uint64_t t2pProfTicks() { return 0; }
#endif
extern thread_local uint64_t g_t2pProf[16];
extern thread_local uint64_t g_t2pStateProf[8];
extern bool g_gxOriginProf; extern int g_gxOrigin; extern uint64_t g_gxOriginCount[2][5];
extern unsigned long g_deferWhy[8];
extern uint32_t g_deferMin;
extern int g_benchRepeat[9];
namespace t2p {

enum : uint32_t {
  kStateDirtyGen = 1u << 0,
  kStateDirtyTev = 1u << 1,
  kStateDirtyTevReg = 1u << 2,
  kStateDirtyPixel = 1u << 3,
  kStateDirtyFog = 1u << 4,
  kStateDirtyView = 1u << 5,
  kStateDirtyTex = 1u << 6,
  kStateDirtyXfMtx = 1u << 7,
  kStateDirtyLight = 1u << 8,
  kStateDirtyChan = 1u << 9,
  kStateDirtyTexGen = 1u << 10,
  kStateDirtyProj = 1u << 11,
  kStateDirtyOther = 1u << 12,
  kStateDirtyVertexFormat = 1u << 13,
  kStateDirtyAll = 0xFFFFFFFFu,
};
extern bool g_stateCpIncremental;
inline uint32_t stateDirtyBp(uint32_t id) {
  if (id == 0x00) return kStateDirtyGen;
  if ((id >= 0x10 && id <= 0x1F) || (id >= 0x28 && id <= 0x2F) || (id >= 0xC0 && id <= 0xDF) || (id >= 0xF6 && id <= 0xFD)) return kStateDirtyTev;
  if (id >= 0xE0 && id <= 0xE7) return kStateDirtyTevReg;
  if ((id >= 0x40 && id <= 0x43) || id == 0xF3) return kStateDirtyPixel;
  if (id >= 0xEE && id <= 0xF2) return kStateDirtyFog;
  if (id == 0x20 || id == 0x21 || id == 0x59) return kStateDirtyView;
  if (id >= 0x80 && id <= 0xBF) return kStateDirtyTex;
  return 0;
}
inline uint32_t stateDirtyXf(uint32_t addr) {
  if (addr < 0x600) return kStateDirtyXfMtx;
  if (addr < 0x700) return kStateDirtyLight;
  if (addr == 0x100A || addr == 0x100C || addr == 0x100E || addr == 0x1010) return kStateDirtyChan;
  if (addr == 0x1012 || addr == 0x1018 || addr == 0x1019 || (addr >= 0x1040 && addr <= 0x1057)) return kStateDirtyTexGen;
  if (addr >= 0x101A && addr <= 0x101F) return kStateDirtyView;
  if (addr >= 0x1020 && addr <= 0x1026) return kStateDirtyProj;
  return 0;
}


struct GxAttrPlan {
  enum Kind : uint8_t { Byte, Pos, Nrm, Clr, Tex } kind;
  uint8_t type;
  uint8_t fmt;
  uint8_t nc;
  bool be;
  int slot = 0;
  int arrayId;
  uint32_t streamBytes;
  uint32_t offset;
  uint32_t need;
  float scale;
  const uint8_t* base;
  uint32_t stride, size;
  uint8_t* out;
  uint32_t outStride;
};


enum GxAttrType : uint8_t { GXA_NONE = 0, GXA_DIRECT = 1, GXA_INDEX8 = 2, GXA_INDEX16 = 3 };
enum GxCompFmt : uint8_t { GXC_U8 = 0, GXC_S8 = 1, GXC_U16 = 2, GXC_S16 = 3, GXC_F32 = 4 };
enum GxClrFmt : uint8_t { GXCL_RGB565 = 0, GXCL_RGB8 = 1, GXCL_RGBX8 = 2, GXCL_RGBA4 = 3, GXCL_RGBA6 = 4, GXCL_RGBA8 = 5 };
enum GxPrim : uint8_t { GXP_QUADS = 0x80, GXP_QUADS2 = 0x88, GXP_TRIANGLES = 0x90, GXP_TRISTRIP = 0x98, GXP_TRIFAN = 0xA0, GXP_LINES = 0xA8, GXP_LINESTRIP = 0xB0, GXP_POINTS = 0xB8 };

enum GxArrayId : uint8_t { GXAR_POS = 0, GXAR_NRM = 1, GXAR_CLR0 = 2, GXAR_CLR1 = 3, GXAR_TEX0 = 4,  GXAR_POSMTX = 12, GXAR_NRMMTX = 13, GXAR_TEXMTX = 14, GXAR_LIGHT = 15, GXAR_COUNT = 16 };

struct GxVcd {
  uint8_t pmIdx = 0;
  uint8_t texMtxIdx[8] = {};
  uint8_t pos = 0, nrm = 0, clr[2] = {}, tex[8] = {};
};
struct GxVat {
  uint8_t posCnt = 1, posFmt = 4, posShift = 0;
  uint8_t nrmCnt = 0, nrmFmt = 4; bool nrmIdx3 = false;
  uint8_t clrCnt[2] = {1, 1}, clrFmt[2] = {5, 5};
  uint8_t texCnt[8] = {1, 1, 1, 1, 1, 1, 1, 1}, texFmt[8] = {4, 4, 4, 4, 4, 4, 4, 4}, texShift[8] = {};
};

struct GxRegs {
  uint32_t bp[256] = {};
  bool bpWritten[256] = {};
  uint32_t cp[256] = {};
  uint32_t xf[0x1100] = {};
  uint32_t tevRaw[2][8] = {};
  uint32_t texAddr[8] = {};
  bool auroraViewport = false;
  uint32_t copyDstAddr = 0;
  uint32_t tlutSrcAddr = 0;
  GxVcd vcd() const;
  GxVat vat(int idx) const;
  float xfFloat(uint32_t addr) const;
};

struct GxArray {
  const uint8_t* data = nullptr;
  uint32_t size = 0;
  uint32_t stride = 0;
  bool bigEndian = true;
  bool strideFromCp = false;
};

struct GxVertices {
  uint32_t count = 0;
  bool hasPos = false, hasNrm = false, hasClr[2] = {}, hasTex[8] = {}, hasPmIdx = false, hasTexMtxIdx[8] = {};
  DecodedVector<float> pos;
  DecodedVector<float> nrm;
  DecodedVector<uint8_t> clr[2];
  DecodedVector<float> tex[8];
  DecodedVector<uint8_t> pmIdx;
  DecodedVector<uint8_t> texMtxIdx[8];
};

struct GxDraw {
  uint8_t prim = 0;
  uint8_t vatIdx = 0;
  uint32_t commandCount = 1;
  uint32_t vertexCount = 0;
  GxVertices v;
  DecodedVector<uint32_t> tris;
  bool unsupportedPrim = false;
  bool retained = false;
  uint32_t retainedTris = 0;
  GxVcd vcd;
  GxVat vat;
};

struct GxDrawInputs {
  uint8_t cmd = 0;
  int partCount = 0;
  const uint8_t* partPrim = nullptr;
  const uint32_t* partCount32 = nullptr;
  size_t partStride = 0;
  uint32_t vertices = 0, vsize = 0;
  const uint8_t* stream = nullptr;
  const uint32_t* fmtRaw = nullptr;
  int nAttrs = 0;
  const GxAttrPlan* attrs = nullptr;
  bool fast = false;
  uint64_t shapeKey = 0;
  uint32_t shapeDraw = 0;
};

inline size_t gxTriCount(const GxDraw& d) { return d.retained ? d.retainedTris : d.tris.size(); }

struct GxUiRoute { bool begin = false; int16_t src[4] = {}; int16_t dst[4] = {}; };

class GxSink {
 public:
  virtual ~GxSink() {}
  virtual void onDraw(const GxRegs&, const GxDraw&) = 0;
  virtual GxDraw* appendTarget(const GxRegs&, const GxDraw& header) { (void)header; return nullptr; }
  virtual void onAppended(const GxRegs&, GxDraw& into, uint32_t firstVertex) { (void)into; (void)firstVertex; }
  virtual bool retainOffer(const GxDrawInputs&, GxDraw& D) { (void)D; return false; }
  virtual void retainDecoded(const GxDrawInputs&, const GxDraw& D, uint32_t firstVertex, size_t firstTri, bool arrayError) { (void)D; (void)firstVertex; (void)firstTri; (void)arrayError; }
  virtual uint32_t primitiveRunLimit() const { return 0; }
  virtual void onDrawDone(const GxRegs&) {}
  virtual void onCopy(const GxRegs&) {}
  virtual void onTlutLoad(const GxRegs&) {}
  virtual void onUiRoute(const GxRegs&, const GxUiRoute&) {}
  virtual void onError(const std::string&) {}
  virtual const uint8_t* resolveDisplayList(uint32_t addr, uint32_t size) { (void)addr; (void)size; return nullptr; }
};

class GxFifo {
 public:
  explicit GxFifo(GxSink* sink = nullptr) : sink_(sink) {}
  void setSink(GxSink* s) { sink_ = s; }
  void setArray(int id, const GxArray& a) { arrays_[id] = a; }
  GxArray& array(int id) { return arrays_[id]; }
  using GuestRead = std::function<const uint8_t*(uint32_t addr, uint32_t size)>;
  void setGuestArrayReader(GuestRead f) { guestRead_ = std::move(f); }
  const GxRegs& regs() const { return regs_; }
  GxRegs& regs() { return regs_; }
  size_t process(const uint8_t* data, size_t size);
  uint64_t drawCount() const { return draws_; }
  uint64_t stateGeneration() const { return stateGen_; }
  void setStaticTracking(bool on){staticTracking_=on;staticRevision_=1;}
  uint32_t staticRevision()const{return staticTracking_&&staticRevision_!=UINT32_MAX?staticRevision_:0;}

  uint32_t takeStateDirty() { const uint32_t d = stateDirty_; stateDirty_ = 0; return d; }
  void setPrimitiveRuns(bool on) { primitiveRuns_ = on; }
  void setGeometryCache(bool on) { if(on&&!decodeCache_)decodeCache_=std::make_unique<GxDecodeCache>();else if(!on)decodeCache_.reset(); }
  GxDecodeCacheStats geometryCacheStats() const { return decodeCache_ ? decodeCache_->stats() : GxDecodeCacheStats{}; }
  void setFastAttributeDecode(bool on) { fastAttributeDecode_ = on; }
  void setDeferredDecode(bool on, void (*wake)() = nullptr);
  bool deferredDecodeActive() const { return bool(deferQ_); }
  void setDeferredDecodePaused(bool on) { deferPaused_ = on; }
  bool runDecodeJob();
  void finishDecodes();
  uint64_t deferredJobs() const { return deferredJobs_; }
  uint64_t deferredSelfRun() const { return deferredSelf_; }
  uint64_t deferredDrains() const { return deferredDrains_; }
  void setCachedVertexPlan(bool on) { cachedVertexPlan_ = on; }
  void setAuroraMode(bool on) { if(aurora_!=on)touchStatic();aurora_ = on; regs_.auroraViewport = on; stateDirty_ = kStateDirtyAll; }
  bool auroraMode() const { return aurora_; }
  void setStopAtDrawDone(bool on) { stopAtDrawDone_ = on; }
  void setUncheckedArrays(bool on) { uncheckedArrays_ = on; }
  bool takeDrawDone() { const bool h = drawDoneHit_; drawDoneHit_ = false; return h; }
  bool processStateProgram(const uint8_t* data, size_t size);
  bool processShapeProgram(uint64_t hint,const uint8_t* data,size_t size);
  uint64_t materialRefs_=0;
  uint64_t materialRefs()const{return materialRefs_;}
  uint64_t modelGroups_=0;
  bool loadPreparedShapes(const char* path){try{auto store=std::make_unique<PreparedShapeStore>();if(!store->open(path))return false;preparedShapes_=std::move(store);return true;}catch(...){return false;}}
  uint64_t preparedShapeHits()const{return preparedShapes_?preparedShapes_->hits:0;}
  uint64_t preparedShapeMisses()const{return preparedShapes_?preparedShapes_->misses:0;}
  uint64_t preparedShapeRuns()const{return preparedShapeRuns_;}
  size_t preparedShapeBytes()const{return preparedShapes_?preparedShapes_->bytes():0;}
  bool processShapeRef(const uint8_t* dl, uint32_t size, uint32_t gen);
  uint64_t shapeRefs() const { return shapeRefs_; }
  uint64_t modelGroups()const{return modelGroups_;}
  void setShapeRefs(bool on) { shapeRefsOn_ = on; }
  uint64_t shapeProgramHits()const{return shapeProgramHits_;}
  uint64_t shapeProgramMisses()const{return shapeProgramMisses_;}
  uint64_t stateProgramHits() const { return stateProgramHits_; }
  uint64_t stateProgramMisses() const { return stateProgramMisses_; }
  void applyProgramBp(uint32_t v,uint32_t dirty) { handleBpPrepared(v,dirty); }
  void applyProgramXf(uint32_t addr,const uint32_t* words,uint32_t n);
  void applyBp(uint32_t v) {
    handleBp(v);
    if ((v >> 24) == 0x45) { finishDecodes(); if (sink_) sink_->onDrawDone(regs_); if (stopAtDrawDone_) drawDoneHit_ = true; }
  }
  void applyCp(uint8_t addr, uint32_t v) { handleCp(addr, v); }
  void applyXf(uint32_t addr, const uint8_t* bigEndianWords, uint32_t n) { handleXf(addr, bigEndianWords, n); }
  void setInputObserver(std::function<void(const GxDrawInputs&)> f) { inputObserver_ = std::move(f); }
  void setRetainOffers(bool on) { retainOffers_ = on; }
  bool retainOffers() const { return retainOffers_; }
  uint64_t processSerial() const { return processSerial_; }

 private:
  void handleBp(uint32_t v);
  void handleBpPrepared(uint32_t v,uint32_t dirty);
  void handleCp(uint8_t addr, uint32_t v);
  void handleXf(uint32_t addr, const uint8_t* words, uint32_t n);
  void indexedXf(int arrayId, uint16_t index, uint16_t addrLen);
  using PrimitiveRunPart=GxPrimitiveRunPart;
  int tryDrawRun(const uint8_t*& p,const uint8_t* end);
  bool handleDraw(uint8_t cmd, const uint8_t*& p, const uint8_t* end, int count = -1, const PrimitiveRunPart* parts = nullptr, int partCount = 0);
  std::unique_ptr<PreparedShapeStore> preparedShapes_;
  uint64_t preparedShapeRuns_=0;
  bool executePreparedShape(const PreparedShape&);
  bool handleAurora(const uint8_t*& p, const uint8_t* end);
  bool aurora_ = false;
  bool stopAtDrawDone_ = false, drawDoneHit_ = false, uncheckedArrays_ = false;
  uint8_t auroraTlutFmt_[32] = {};
  uint32_t auroraCopyBits_ = 0;
  bool auroraCopyZ_ = false;
  void err(const std::string& s) { if (sink_) sink_->onError(s); }

  std::array<std::unique_ptr<GxStateProgram>,128> statePrograms_;
  std::array<uint64_t,128> stateProgramAges_{};
  uint64_t stateProgramClock_=0;
  size_t stateProgramBytes_=0;
  uint64_t stateProgramHits_=0, stateProgramMisses_=0;
  struct ShapeProgram {
    struct Part {uint32_t offset,count,end;uint8_t cmd;};
    struct Zero {uint32_t offset,size;};
    uint64_t hint=0;size_t size=0;uint32_t format[5]={};uint8_t vat=0;
    std::vector<Part> parts;std::vector<Zero> zeros;
    size_t cost()const{return sizeof(*this)+parts.capacity()*sizeof(Part)+zeros.capacity()*sizeof(Zero);}
  };
  std::array<std::unique_ptr<ShapeProgram>,64> shapePrograms_;
  size_t shapeProgramBytes_=0;
  uint64_t shapeProgramHits_=0,shapeProgramMisses_=0;
  uint64_t shapeKey_=0,shapeRefs_=0; uint32_t shapeDraw_=0; bool shapeRefsOn_=true;
  GxSink* sink_ = nullptr;
  std::function<void(const GxDrawInputs&)> inputObserver_;
  bool retainOffers_ = false;
  uint64_t processSerial_ = 0;
  int processDepth_ = 0;
  GxRegs regs_;
  GxArray arrays_[GXAR_COUNT];
  GuestRead guestRead_;
  const uint8_t* guestElement(int arrayId, uint32_t index, uint32_t need) const;
  uint64_t draws_ = 0;
  uint32_t bpMask_ = 0xFFFFFF;
  uint64_t stateGen_ = 0;
  GxDraw scratch_;
  struct FmtCache { bool valid = false; uint32_t raw[5] = {}; GxVcd vcd; GxVat vat; uint32_t vsize = 0; bool planned = false; int nAttrs = 0; GxAttrPlan attrs[21]; };
  bool primitiveRuns_ = false;
  std::array<PrimitiveRunPart,128> runParts_;
  std::vector<uint8_t> runBytes_;
  bool splitRun_ = false;
  void uiRouteMarker(const char* text, uint32_t n);
  std::unique_ptr<GxDecodeCache> decodeCache_;
  bool fastAttributeDecode_ = true;
  struct DecodeQueue;
  struct DecodeQueueDelete { void operator()(DecodeQueue*) const; };
  std::unique_ptr<DecodeQueue, DecodeQueueDelete> deferQ_;
  bool deferPaused_ = false;
  uint64_t deferredJobs_ = 0, deferredSelf_ = 0, deferredDrains_ = 0;
  bool cachedVertexPlan_ = true;
  FmtCache fmt_[8];
  FmtCache& vertexFormat(int vatIdx);
  bool staticTracking_=false;uint32_t staticRevision_=1;
  void touchStatic(){if(staticTracking_&&staticRevision_!=UINT32_MAX)++staticRevision_;}
  void setReg(uint32_t& r, uint32_t v, uint32_t group) {
    if(r!=v){
      constexpr uint32_t groups=kStateDirtyGen|kStateDirtyTev|kStateDirtyTevReg|kStateDirtyPixel|kStateDirtyFog|kStateDirtyView|kStateDirtyTex|kStateDirtyChan|kStateDirtyTexGen|kStateDirtyProj;
      if((group&groups)&&staticTracking_){
        const bool colour=&r==&regs_.xf[0x100a]||&r==&regs_.xf[0x100c];
        const bool selected=&r==&regs_.xf[0x1018]&&((r^v)&~63u)==0;
        if(!colour&&!selected)touchStatic();
      }
      r=v;stateGen_++;stateDirty_|=group;
    }
  }
  uint32_t stateDirty_ = kStateDirtyAll;
};

uint32_t gxVertexSize(const GxVcd&, const GxVat&);
uint32_t gxTexBufferSize(uint32_t w, uint32_t h, uint32_t fmt, bool mips);

}

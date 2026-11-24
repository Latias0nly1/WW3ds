#pragma once
#include "clip_bounds.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "gxfifo.h"

namespace t2p {

struct GeometryPmMemo { int8_t state = 0; uint8_t why = 0, n = 0; int8_t k[10] = {}; };
struct RetainedGeometry {
  GxDraw d;
  mutable ClipBounds clipBounds;
  mutable GeometryPmMemo pmMemo;
  uint64_t id = 0;
  int maxPm = -1;
  size_t bytes = 0;
};

struct GxRetainStats {
  uint64_t draws = 0, skipped = 0, batches = 0, retainedBatches = 0, published = 0, materialized = 0, notRetainable = 0, closedOff = 0, overBudget = 0;
  uint64_t verifyBytes = 0, snapshotBytes = 0, streamBytes = 0;
  uint64_t shapeMatches = 0;
  uint64_t shapeOffered = 0, shapeNewNoPrev = 0, shapeNewMiss = 0, shapeMidMiss = 0;
  uint64_t missBatches[9] = {}, missVerts[9] = {};
  size_t heldBytes = 0;
};

class GxRetainer {
 public:
  GxRetainer();
  ~GxRetainer();
  void setBudget(size_t bytes) { budget_ = bytes; }

  bool offer(const GxDrawInputs& in, GxDraw& D, bool newBatch, uint64_t chunk, int& drawMaxPm);
  void decoded(const GxDrawInputs& in, const GxDraw& D, uint32_t firstVertex, size_t firstTri, bool arrayError, uint64_t chunk);
  std::shared_ptr<const RetainedGeometry> closeBatch(GxDraw& D, int maxPm);
  bool open() const { return open_; }
  std::shared_ptr<const RetainedGeometry> matchingGeometry()const{return mode_==Matching&&cand_?cand_->geo:nullptr;}
  void endFrame();
  void clear();
  GxRetainStats takeStats() { GxRetainStats s = st_; st_ = GxRetainStats(); s.heldBytes = held_; return s; }

 private:
  struct Watch;
  struct Binding { const uint8_t* base; uint32_t stride, size; bool be; Watch* w; uint32_t version; };
  struct Range { uint32_t binding; uint32_t b0, b1; };
  struct DrawRec {
    uint8_t cmd = 0; uint8_t nRange = 0; uint32_t vertices = 0, vsize = 0; uint32_t fmt[5] = {};
    uint32_t partOff = 0, nParts = 0;
    uint32_t streamOff = 0;
    uint32_t rangeOff = 0;
    int maxPm = -1;
    uint32_t vertsEnd = 0; uint32_t trisEnd = 0;
    uint64_t shapeKey = 0; uint32_t shapeDraw = 0;
  };
  struct Batch {
    uint64_t key = 0;
    std::vector<DrawRec> recs;
    std::vector<uint32_t> parts;
    std::vector<uint8_t> stream;
    std::vector<Binding> bindings;
    std::vector<Range> ranges;
    std::shared_ptr<RetainedGeometry> geo;
    uint64_t usedFrame = ~0ull;
    size_t bytes = 0;
  };
  enum Mode { None, Matching, Fresh, Off };

  static bool retainable(const GxDrawInputs& in);
  static uint64_t keyOf(const GxDrawInputs& in);
  bool match(const Batch& b, const DrawRec& r, const GxDrawInputs& in, uint64_t chunk);
  bool verify(const Binding& bd, uint32_t b0, uint32_t b1, uint64_t chunk);
  Watch* snapshot(const uint8_t* base, uint32_t b0, uint32_t b1, uint64_t chunk, uint32_t& version);
  void record(const GxDrawInputs& in, const GxDraw& D, uint32_t firstVertex, uint64_t chunk);
  void materialize(GxDraw& D);
  void use(const std::shared_ptr<Batch>& b);

  std::unordered_map<uint64_t, std::vector<std::shared_ptr<Batch>>> prev_;
  std::vector<std::shared_ptr<Batch>> cur_;
  std::unordered_map<const uint8_t*, std::unique_ptr<Watch>> watches_;
  bool open_ = false;
  Mode mode_ = None;
  std::shared_ptr<Batch> cand_;
  uint32_t cursor_ = 0;
  std::shared_ptr<Batch> building_;
  uint64_t frame_ = 0, nextId_ = 0;
  size_t budget_ = 8u << 20, held_ = 0, heldNext_ = 0;
  GxRetainStats st_;
  uint8_t lastFail_ = 0, reason_ = 0;
};

}

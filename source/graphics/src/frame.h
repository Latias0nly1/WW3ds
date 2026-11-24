#pragma once
#include <string>
#include <vector>

#include "gxstate.h"
#include "pipeline.h"

namespace t2p {

enum class FitMode { Pillarbox43, Stretch, Explicit };

struct FrameConfig {
  int efbW = 640, efbH = 448;
  int targetW = 400, targetH = 240;
  FitMode fit = FitMode::Pillarbox43;
  bool rotate = true;
  bool widePicture = false;
  float outX = 0, outY = 0, outW = 400, outH = 240;
  void resolve();
};

struct PixelRect { int x0 = 0, y0 = 0, x1 = 0, y1 = 0; bool empty() const { return x1 <= x0 || y1 <= y0; } };

struct PicaFrame {
  bool ok = false;
  float post[4][4] = {};
  float proj[4][4] = {};
  bool mirrored = false;
  int picaCull = 0;
  PixelRect scissorLogical;
  PixelRect scissorWindow;
  PicaDepthMap depth;
  std::vector<std::string> notes;
};

PicaFrame mapFrame(const GxState& S, const FrameConfig& cfg, const PicaDepthMap& drawDepth, float xScale = 1.0f);
float widescreenXScale(const GxState& S, const GxDraw& d, const FrameConfig& cfg);

void mulMtx4(const float a[4][4], const float b[4][4], float out[4][4]);
PicaDepthMap depthMapZRange(float nearZ, float farZ);

}

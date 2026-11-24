#pragma once
#include <cstdint>
namespace t2p {
struct GxTexBinding {
  bool valid = false;
  uint32_t addr = 0;
  int width = 0, height = 0;
  int fmt = 0;
  uint8_t wrapS = 0, wrapT = 0;
  bool magLinear = false, minLinear = false;
  uint8_t mipMode = 0;
  float lodBias = 0, minLod = 0, maxLod = 0;
  bool edgeLod = false, biasClamp = false;
  uint8_t aniso = 0;
  uint32_t tlutOffset = 0;
  int tlutFmt = 0;
  int mipLevels() const;
};

}

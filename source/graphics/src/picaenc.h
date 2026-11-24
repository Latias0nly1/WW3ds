#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace t2p {

struct PicaBinUniform {
  std::string name;
  int pos = 0;
  int size = 1;
  int type = 2;
};

struct PicaBinConstant {
  int regId = 0;
  int type = 2;
  float f[4] = {0, 0, 0, 0};
  uint8_t i[4] = {0, 0, 0, 0};
  bool b = false;
};

struct PicaBinOutput {
  int type = 0, reg = 0, mask = 0;
};

struct PicaAsmResult {
  bool ok = false;
  std::string error;
  std::vector<std::string> warnings;
  std::vector<uint8_t> shbin;
  std::vector<uint32_t> code;
  std::vector<uint32_t> opdescs;
  std::vector<PicaBinUniform> uniforms;
  std::vector<PicaBinConstant> constants;
  std::vector<PicaBinOutput> outputs;
  uint32_t entryStart = 0, entryEnd = 0;
  uint16_t inputMask = 0, outputMask = 0;
};

PicaAsmResult assemblePica(const std::string& source, const std::string& filename = "shader.v.pica", bool autoNop = true);

uint32_t picaF32toF24(float f);
float picaF24toF32(uint32_t v);

}

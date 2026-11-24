#include "material_vector.h"
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "picaasm.h"

namespace t2p {

struct VtxChan {
  uint8_t enable = 0;
  uint8_t matSrc = 0;
  uint8_t lightMask = 0;
  uint8_t diffuse = 2;
  uint8_t attn = 2;
  uint8_t ambSrc = 0;
};

struct VtxTexGen {
  uint8_t type = 1;
  uint8_t src = 4;
  uint8_t mtx = 60;
  bool normalize = false;
  uint8_t postMtx = 125;
};

struct VtxSetup {
  VtxChan color0, alpha0;
  bool needColor = true;
  MaterialVector<VtxTexGen> texgen;
  int numPosMtx = 1;
  int inPos = 0, inNrm = -1, inClr0 = -1, inMtxIdx = -1;
  int inTex[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  bool flipT = false;
  uint8_t texScaleMask = 0;
  int dbgClipReg = -1;
  int dbgTc = -1;
};

struct VtxShader {
  bool ok = false;
  std::string source;
  std::string error;
  std::vector<std::string> notes;
  bool exact = true;
  int lightSlot[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int numLights = 0;
  int texMtxSlot[3] = {-1, -1, -1};
  int numTexMtx = 0;
  int texScaleSlot[3] = {-1, -1, -1};
  int numTexScale = 0;
  bool tc0Projective = false;
  bool usesNormal = false;
  bool usesPos = false, usesNrmAttr = false, usesClrAttr = false, usesMtxIdxAttr = false, usesTexAttr[8] = {};
  int uniformRegs = 0;
};
VtxShader genVertexShader(const VtxSetup&);

struct VtxLight { float color[4] = {0, 0, 0, 0}, pos[3] = {0, 0, 0}, dir[3] = {0, 0, 1}, cosatt[3] = {1, 0, 0}, distatt[3] = {1, 0, 0}; };
struct VtxUniforms {
  float proj[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
  float pos[10][3][4] = {};
  float nrm[10][3][4] = {};
  float matColor[4] = {255, 255, 255, 255}, ambColor[4] = {0, 0, 0, 255};
  VtxLight light[8];
  float texMtx[10][3][4] = {};
  float texScale[3][2] = {{1, 1}, {1, 1}, {1, 1}};
};
struct VtxUniformView {
 const float (&proj)[4][4];const float (*pos)[3][4];const float (*nrm)[3][4];
 const float (&matColor)[4];const float (&ambColor)[4];const VtxLight (&light)[8];
 const float (&texMtx)[10][3][4];const float (&texScale)[3][2];
 int poseCount=10;
};
inline VtxUniformView uniformViewOf(const VtxUniforms& u){return {u.proj,u.pos,u.nrm,u.matColor,u.ambColor,u.light,u.texMtx,u.texScale};}
struct VtxInput {
  float pos[3] = {0, 0, 0}, nrm[3] = {0, 0, 1}, clr0[4] = {255, 255, 255, 255}, tex[8][2] = {};
  int mtxIdx = 0;
};
struct VtxOutput {
  float clip[4] = {0, 0, 0, 1};
  float color[4] = {1, 1, 1, 1};
  float tc[3][3] = {};
};
VtxOutput evalVertex(const VtxSetup&, const VtxUniforms&, const VtxInput&);

struct PackedUniformRows { PVec rows[40]; int count = 0; };
struct PackedUniforms { PackedUniformRows arrays[8]; };
int packedUniformSlot(const std::string& name);
void packUniformsPacked(PackedUniforms&, const VtxSetup&, const VtxShader&, const VtxUniforms&, const float proj[4][4], const float scale[3][2]);
void packUniformsPacked(PackedUniforms&, const VtxSetup&, const VtxShader&, const VtxUniformView&, const float proj[4][4], const float scale[3][2]);
using UniformMap = std::map<std::string, std::vector<PVec>>;
UniformMap packUniforms(const VtxSetup&, const VtxShader&, const VtxUniforms&);
void packUniformsInto(UniformMap& m, const VtxSetup&, const VtxShader&, const VtxUniforms&);
void packInputs(const VtxSetup&, const VtxInput&, PVec in[16]);

}

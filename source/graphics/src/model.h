#include "material_vector.h"
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace t2p {

struct V3 { float v[3] = {0, 0, 0}; };
struct Rgba { float v[4] = {0, 0, 0, 0}; };

enum Leaf : uint8_t {
  L_CPREV, L_APREV, L_C0, L_A0, L_C1, L_A1, L_C2, L_A2,
  L_TEXC, L_TEXA, L_RASC, L_RASA, L_ONE, L_HALF, L_ZERO, L_KONST,
  L_MID,
  L_PBUF,
  L_PBUFA,
  L_KONSTC,
  L_MIDA,
  L_COUNT
};

struct KSel {
  enum Kind : uint8_t { Fixed, Rgb, Chan } kind = Fixed;
  float fixed = 0;
  uint8_t idx = 0, chan = 0;
};

struct TevChan {
  Leaf in[4] = {L_ZERO, L_ZERO, L_ZERO, L_ZERO};
  bool sub = false;
  float bias = 0, scale = 1;
  bool clamp = true;
  uint8_t out = 0;
  uint8_t cmp = 0;
};

struct TevStage {
  TevChan c, a; KSel kc, ka; int texmap = -1; int rasChan = 0;
  int texcoord = -1;
  uint8_t texSwap[4] = {0, 1, 2, 3}, rasSwap[4] = {0, 1, 2, 3};
  bool regBufC[3] = {false, false, false}, regBufA[3] = {false, false, false};
};
struct TevMaterial { MaterialVector<TevStage> stages; };

struct Inputs { Rgba tex[8], ras[2], kcolor[4], reg[3]; Rgba prev; };

using LeafTable = std::array<V3, L_COUNT>;
LeafTable buildLeaves(const Rgba regs[4], const Inputs&, const TevStage&, bool alphaPath);
V3 evalTevChan(const TevChan&, const LeafTable&);
float snap8(float);
bool tevCompareGT(const TevChan&, const LeafTable&);
inline void setAlphaR8Operands(TevStage& st) {
  for (int j = 0; j < 2; j++) st.a.in[j] = st.c.in[j] == L_KONST ? L_KONSTC : st.c.in[j];
}
Rgba evalTev(const TevMaterial&, const Inputs&);

enum PSrc : uint8_t { P_PRIMARY, P_TEX0, P_TEX1, P_TEX2, P_CONST, P_PREV, P_BUF };
enum POp : uint8_t {
  OP_REPLACE, OP_MODULATE, OP_ADD, OP_ADDSIGNED, OP_LERP, OP_SUB, OP_MAD, OP_ADDMUL, OP_COUNT
};
enum POpr : uint8_t { OPR_RGB, OPR_1RGB, OPR_A, OPR_1A, OPR_R, OPR_1R, OPR_G, OPR_1G, OPR_B, OPR_1B };

struct PArg { PSrc src = P_PREV; POpr opr = OPR_RGB; bool snap = false; };
struct PChan { POp op = OP_REPLACE; PArg arg[3]; uint8_t scale = 1; };

enum { SYM_NONE = 0, SYM_K0 = 1, SYM_R0 = 5 };
struct KComp {
  bool set = false;
  uint8_t sym = SYM_NONE, chan = 0;
  bool inv = false;
  float fixed = 0;
};
struct PStage { PChan c, a; KComp k[4]; bool updBufC = false, updBufA = false; };
struct PicaProgram { std::vector<PStage> stages; };

V3 picaChanEval(POp, const V3 args[3], uint8_t scale, bool quantize);
Rgba resolveConst(const PStage&, const Inputs&, bool quantize = true);
Rgba evalPica(const PicaProgram&, const Inputs&, bool quantize = true, bool globalStart = true,
              const Rgba* localBuf = nullptr);
uint32_t encodeBufferUpdate(const PicaProgram&);

struct HwStage { uint32_t sources, modifiers, ops, constColor, scales; };
HwStage encodeStage(const PStage&, const Rgba& resolvedConst);
extern const uint16_t kPicaTexEnvReg[6];

}

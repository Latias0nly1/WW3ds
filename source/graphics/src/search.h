#pragma once
#include <string>
#include <vector>

#include "alloc.h"
#include "translate.h"

namespace t2p {

struct ArgSel { Leaf leaf = L_ZERO; bool inv = false; uint8_t cmp = 0; };
struct Step { POp op = OP_REPLACE; ArgSel arg[3]; uint8_t scale = 1; };
struct ChanSeq { std::vector<Step> steps; double err = 0; };

struct SrcInfo { PSrc src = P_PREV; POpr opr = OPR_RGB; bool isConst = false; Demand dem; };
bool leafSource(Leaf, bool inv, bool alpha, const TevStage&, SrcInfo&, std::string& why, bool red = false);
enum { PREVBUF_RGB = 1, PREVBUF_ALPHA = 2 };
unsigned prevBufNeeds(const ChanSeq&, bool alphaChannel);
extern const int kArity[OP_COUNT];

struct ChanResult {
  Fidelity f = Fidelity::Unsupported;
  std::vector<ChanSeq> seqs;
  std::vector<Step> mask;
  std::string note;
};
ChanResult solveChan(const TevStage&, bool alpha, double approxLimit, uint64_t seed, bool hoistMask = false, int minSteps = 1);

}

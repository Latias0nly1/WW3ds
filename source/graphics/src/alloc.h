#pragma once
#include "model.h"

namespace t2p {

struct Demand {
  bool vec = false;
  bool fixed = false;
  float val = 0;
  uint8_t sym = SYM_NONE, chan = 0;
  bool inv = false;
};

struct Alloc {
  KComp k[4];
  std::vector<POpr> opr;
};

bool allocate(const std::vector<Demand>&, Alloc&);

}

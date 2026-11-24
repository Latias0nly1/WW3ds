#pragma once
#include <algorithm>
#include <cstdint>
namespace n3ds {
struct PaintBudget {
  int64_t credit=0;
  bool decide(int64_t period,int64_t cost,bool ready,bool forced,bool catchUp){
    cost=std::max(cost,period);
    credit=std::min(credit+period,cost+period);
    const bool paint=(ready && credit>=cost && !catchUp)||forced;
    if(paint)credit=std::max(-cost,credit-cost);
    return paint;
  }
};
}

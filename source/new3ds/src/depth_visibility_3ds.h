#pragma once
#include <algorithm>
#include <cmath>
#include "picaenc.h"
#include "pipeline.h"
struct DepthVisibility3ds {
  struct Camera {float a,b,nearPlane,farPlane,nearZ,farZ;bool ortho;};
  Camera camera[16]{};unsigned count=0;bool complete=true;
  float fogRange=0;
  void add(Camera c){
    for(unsigned i=0;i<count;i++){const auto& x=camera[i];if(x.a==c.a&&x.b==c.b&&x.nearPlane==c.nearPlane&&x.farPlane==c.farPlane&&x.nearZ==c.nearZ&&x.farZ==c.farZ&&x.ortho==c.ortho)return;}
    if(count==16){complete=false;return;}camera[count++]=c;
  }
  bool isFar(uint32_t raw)const{
    raw&=0xFFFFFFu;
    if(t2p::picaReverseZ())return raw<=0x100u;
    if(raw>=0xFFBBFFu)return true;
    if(!complete||!(fogRange>0))return false;
    constexpr double maximum=16777215.0;
    bool compatible=false;
    for(unsigned i=0;i<count;i++){
      const auto& c=camera[i];
      if(!(c.farZ>0)||!std::isfinite(c.a)||!std::isfinite(c.b))return false;
      const double scale=t2p::picaF24toF32(t2p::picaF32toF24(c.farZ*(255.0f/256.0f)/fogRange));
      if(!(scale>0))return false;
      double lo=std::max(0.0,(double(raw)-1.0)/maximum)/scale;
      double hi=(double(raw)+1.0)/maximum/scale;
      if(c.ortho){if(lo<=1&&hi>=1)return false;continue;}
      if(!(c.nearPlane>0&&c.farPlane>c.nearPlane&&c.b<0&&c.farZ>c.nearZ))return false;
      const double tolerance=double(c.farPlane)*0.0001;
      if(hi<c.nearPlane-tolerance||lo>c.farPlane+tolerance)continue;
      compatible=true;
      const double whi=std::min(std::max(hi*(1.0+1.0/4096.0),double(c.nearPlane)),double(c.farPlane));
      const double gx=c.nearZ+(c.farZ-c.nearZ)*(1.0-c.a+c.b/whi);
      if(!(gx>=1.0-0.5/maximum))return false;
    }
    return compatible;
  }
};

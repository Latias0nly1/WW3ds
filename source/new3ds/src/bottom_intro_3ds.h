#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <3ds.h>
#include "n3ds.h"

namespace n3ds_intro {
struct Clip {
  FILE* file=nullptr;
  uint16_t fps=12;
  uint32_t count=0;
  uint32_t next=0xffffffffu;
  long data=0;
  bool failed=false;
  bool done=false;
};
inline Clip& clip(){static Clip c; return c;}
inline bool active(){return !clip().done;}
inline void dismiss(){
  Clip& c=clip();
  c.done=true;
  if(c.file){std::fclose(c.file);c.file=nullptr;}
}
inline void open(){
  Clip& c=clip();
  if(c.file||c.failed||c.done)return;
  c.file=std::fopen(N3DS_ROOT "/bottom_intro.wwbv","rb");
  if(!c.file){c.failed=true;return;}
  char magic[4]; uint16_t ver=0,w=0,h=0;
  if(std::fread(magic,1,4,c.file)!=4||std::memcmp(magic,"WWBV",4)||
     std::fread(&ver,2,1,c.file)!=1||ver!=1||
     std::fread(&w,2,1,c.file)!=1||std::fread(&h,2,1,c.file)!=1||w!=320||h!=240||
     std::fread(&c.fps,2,1,c.file)!=1||std::fread(&c.count,4,1,c.file)!=1||!c.count||!c.fps){
    std::fclose(c.file);c.file=nullptr;c.failed=true;return;
  }
  c.data=std::ftell(c.file);
}
inline bool blit(uint16_t* dst){
  open();
  Clip& c=clip();
  if(!c.file)return false;
  static uint64_t t0=0;
  if(!t0)t0=svcGetSystemTick();
  const uint32_t frame=uint32_t(((svcGetSystemTick()-t0)*uint64_t(c.fps))/SYSCLOCK_ARM11)%c.count;
  if(frame!=c.next){
    if(std::fseek(c.file,c.data+long(frame)*320*240*2,SEEK_SET)!=0)return false;
  }
  if(std::fread(dst,2,320*240,c.file)!=320*240){std::clearerr(c.file);return false;}
  c.next=(frame+1)%c.count;
  GSPGPU_FlushDataCache(dst,320*240*2);
  return true;
}
}

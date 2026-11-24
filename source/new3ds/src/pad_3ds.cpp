#include <dolphin/pad.h>
#include <dolphin/si.h>

#include <cstring>
#include <atomic>

#include "n3ds.h"
#include "button_remap_3ds.h"
#include "ui_shortcuts_api_3ds.h"
#include "qol_config_3ds.h"
#include "first_boot_gate_3ds.h"
extern "C" void pc_aim_gyro_set(int) __attribute__((weak));
extern "C" void pc_aim_gyro_sens_set(float) __attribute__((weak));
extern "C" int pc_qol_set(const char*,const char*) __attribute__((weak));
extern "C" uint32_t n3dsUiTouchAction(uint32_t, int) __attribute__((weak));
extern "C" int n3dsUiLoadShortcuts(void) __attribute__((weak));
extern "C" void n3dsCompassSample(void) __attribute__((weak));
extern "C" void n3dsUiToggleMapBottom(void) __attribute__((weak));

extern "C" int n3dsMotionEnable(int,int) __attribute__((weak));
extern "C" int n3dsMotionRead(int,float*,int) __attribute__((weak));
extern "C" void n3dsMotionStop(void) __attribute__((weak));
namespace {
std::atomic<bool> sUpperHud{true};
std::atomic<unsigned> sMotionFlip{0};
bool sPadReady = false;
bool sPadHeld = false;
PADStatus sGhostPad[PAD_CHANMAX];
bool sGhostPadOn[PAD_CHANMAX];

s8 axis(int v, int full) {
  int x = v * 100 / full;
  if (x > 127) x = 127;
  if (x < -128) x = -128;
  if (x > -6 && x < 6) x = 0;
  return s8(x);
}

void ClampStick(s8* px, s8* py, s8 max, s8 xy, s8 min) {
  int x = *px, y = *py, signX = 1, signY = 1;
  if (x < 0) { signX = -1; x = -x; }
  if (y < 0) { signY = -1; y = -y; }
  if (x <= min) x = 0; else x -= min;
  if (y <= min) y = 0; else y -= min;
  if (x == 0 && y == 0) { *px = *py = 0; return; }
  if (xy * y <= xy * x) {
    const int d = xy * x + (max - xy) * y;
    if (xy * max < d) { x = s8(xy * max * x / d); y = s8(xy * max * y / d); }
  } else {
    const int d = xy * y + (max - xy) * x;
    if (xy * max < d) { x = s8(xy * max * x / d); y = s8(xy * max * y / d); }
  }
  *px = s8(signX * x);
  *py = s8(signY * y);
}

void ClampTrigger(u8* t, u8 min, u8 max) {
  if (*t <= min) *t = 0;
  else { if (max < *t) *t = max; *t -= min; }
}
}

extern "C" {

void n3dsMotionAdjustTurn(float* yaw,float* pitch) {
 const unsigned flags=sMotionFlip.load(std::memory_order_relaxed);
 if(yaw&&(flags&1))*yaw=-*yaw;
 if(pitch&&(flags&2))*pitch=-*pitch;
}
void n3dsUiSetTopHudEnabled(int on) { sUpperHud.store(on!=0,std::memory_order_release); n3dsLog("[perf] top_hud %d\n",int(on!=0)); }
int n3dsUiTopHudEnabled() { return sUpperHud.load(std::memory_order_acquire); }

BOOL PADInit() {
 n3ds_buttons::init();
 if(!sPadReady&&pc_qol_set){
  n3ds_qol::Profile profile;
  const int result=n3ds_qol::load(N3DS_ROOT "/qol.ini",profile);
  if(result==0){
   sMotionFlip.store((profile.motionFlipX?1u:0u)|(profile.motionFlipY?2u:0u),std::memory_order_relaxed);
   if(pc_aim_gyro_set)pc_aim_gyro_set(profile.motionAim);
   if(pc_aim_gyro_sens_set)pc_aim_gyro_sens_set(float(profile.motionScale)/100.0f);
   n3dsLog("[qol] gyro %d sensitivity %d invert %u\n",int(profile.motionAim),profile.motionScale,sMotionFlip.load(std::memory_order_relaxed));
   pc_qol_set("COS_QOL_SAIL",profile.sailMode);
   pc_qol_set("COS_QOL_FAST_GRAPPLE",profile.fastGrapple?"1":"0");
   pc_qol_set("COS_QOL_NO_SONG_REPLAY",profile.skipSongReplay?"1":"0");
   pc_qol_set("COS_QOL_INSTANT_TEXT",profile.instantLines?"1":"0");
   n3dsLog("[qol] sail %s grapple %d songs %d text %d\n",profile.sailMode,int(profile.fastGrapple),int(profile.skipSongReplay),int(profile.instantLines));
  }else if(result!=-1)n3dsLog("[qol] qol.ini rejected at line %d; existing settings retained\n",result);
 }

 if(!sPadReady&&n3dsUiLoadShortcuts){const int result=n3dsUiLoadShortcuts();
  if(result!=-1&&result!=0)n3dsLog("[ui] shortcuts.ini rejected (%d); defaults retained\n",result);
 }
 sPadReady = true;return TRUE;
}
BOOL PADReset(u32) { return TRUE; }
BOOL PADRecalibrate(u32) { return TRUE; }
void PADSetSpec(u32) {}
void PADSetAnalogMode(u32) {}
void PADControlMotor(u32, u32) {}
void PADControlAllMotors(const u32*) {}
void PADBlockInput(bool block) { sPadHeld = block; if(block&&n3dsMotionStop)n3dsMotionStop(); }
u32 PADCount() { return 1; }

u32 PADRead(PADStatus* status) {
  for (u32 i = 0; i < PAD_CHANMAX; i++) {
    std::memset(&status[i], 0, sizeof(PADStatus));
    status[i].err = i == 0 ? PAD_ERR_NONE : PAD_ERR_NO_CONTROLLER;
  }
  N3dsPad p;
  n3dsHidRead(&p);
  const u32 touch = n3dsUiTouchAction ? n3dsUiTouchAction(p.keys, sPadHeld || sGhostPadOn[0]) : 0;
  if (!sPadHeld && !n3ds_warm::blockingInput()) {
    PADStatus& s = status[0];
    s.button = n3ds_buttons::mapping.apply(p.keys);
    if(s.button & PAD_TRIGGER_L)s.triggerLeft=255;
    if(s.button & PAD_TRIGGER_R)s.triggerRight=255;
    if(touch & N3DS_UI_ITEM_X)s.button|=PAD_BUTTON_X;
    if(touch & N3DS_UI_ITEM_Y)s.button|=PAD_BUTTON_Y;
    if(touch & N3DS_UI_ITEM_Z)s.button|=PAD_TRIGGER_Z;
    static u32 touchPrev=0;
    if((touch&N3DS_UI_MAP_SIZE)&&!(touchPrev&N3DS_UI_MAP_SIZE)&&n3dsUiToggleMapBottom)n3dsUiToggleMapBottom();
    touchPrev=touch;
    s.stickX = axis(p.stickX, 150);
    s.stickY = axis(p.stickY, 150);
    s.substickX = axis(p.cstickX, 150);
    s.substickY = axis(p.cstickY, 150);
    if (s.button & PAD_BUTTON_A) s.analogA = 255;
    if (s.button & PAD_BUTTON_B) s.analogB = 255;
  }
  for (u32 i = 0; i < PAD_CHANMAX; i++)
    if (sGhostPadOn[i]) status[i] = sGhostPad[i];
  if(n3dsCompassSample)n3dsCompassSample();
  return 0;
}

void PADClamp(PADStatus* status) {
  for (u32 i = 0; i < PAD_CHANMAX; i++) {
    if (status[i].err != PAD_ERR_NONE) continue;
    ClampStick(&status[i].stickX, &status[i].stickY, 72, 40, 15);
    ClampStick(&status[i].substickX, &status[i].substickY, 59, 31, 15);
    ClampTrigger(&status[i].triggerLeft, 30, 180);
    ClampTrigger(&status[i].triggerRight, 30, 180);
  }
}
void PADClampCircle(PADStatus* status) { PADClamp(status); }

void PADSetVirtualStatus(u32 port, const PADStatus* status) {
  if (port < PAD_CHANMAX) { sGhostPad[port] = *status; sGhostPadOn[port] = true; if(port==0&&n3dsMotionStop)n3dsMotionStop(); }
}
void PADClearVirtualStatus(u32 port) {
  if (port < PAD_CHANMAX) sGhostPadOn[port] = false;
}
void PADClearAllVirtualStatus() {
  for (u32 i = 0; i < PAD_CHANMAX; i++) sGhostPadOn[i] = false;
}

const char* PADGetName(u32 port) { return port == 0 ? "Nintendo 3DS" : nullptr; }
const char* PADGetNameForControllerIndex(u32 idx) { return idx == 0 ? "Nintendo 3DS" : nullptr; }
s32 PADGetIndexForPort(u32 port) { return port == 0 ? 0 : -1; }
void PADSetPortForIndex(u32, u32) {}
void PADClearPort(u32) {}
void PADGetVidPid(u32, u32* vid, u32* pid) { *vid = 0; *pid = 0; }
BOOL PADIsGCAdapter(u32) { return FALSE; }
BOOL PADHasSensor(u32 port, PADSensorType sensor) {
 return port==0&&(sensor==PAD_SENSOR_ACCEL||sensor==PAD_SENSOR_GYRO)&&n3dsMotionEnable&&n3dsMotionRead;
}
BOOL PADSetSensorEnabled(u32 port, PADSensorType sensor, BOOL on) {
 if(!PADHasSensor(port,sensor)||sPadHeld||sGhostPadOn[0])return FALSE;
 return n3dsMotionEnable(int(sensor),on!=0)?TRUE:FALSE;
}
BOOL PADGetSensorData(u32 port, PADSensorType sensor, f32* out, int count) {
 if(!PADHasSensor(port,sensor)||!out||count<3||sPadHeld||sGhostPadOn[0])return FALSE;
 return n3dsMotionRead(int(sensor),out,count)?TRUE:FALSE;
}
BOOL PADHasLED(u32) { return FALSE; }
BOOL PADSetColor(u32, u8, u8, u8) { return FALSE; }
BOOL PADGetColor(u32, u8*, u8*, u8*) { return FALSE; }
BOOL PADSetRumbleIntensity(u32, u16, u16) { return FALSE; }
BOOL PADGetRumbleIntensity(u32, u16* l, u16* h) { *l = *h = 0; return FALSE; }
BOOL PADSupportsRumbleIntensity(u32) { return FALSE; }
BOOL PADCanForceDeviceRumble(u32) { return FALSE; }
BOOL PADGetForceDeviceRumble(u32) { return FALSE; }
BOOL PADSetForceDeviceRumble(u32, BOOL) { return FALSE; }

u32 SIProbe(s32 chan) { return chan == 0 ? 0x09000000u : 0x00000008u; }

}

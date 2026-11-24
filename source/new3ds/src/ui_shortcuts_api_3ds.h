#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
enum N3dsUiShortcut {
 N3DS_UI_NONE=0, N3DS_UI_ITEM_X=1u<<0, N3DS_UI_ITEM_Y=1u<<1,
 N3DS_UI_ITEM_Z=1u<<2, N3DS_UI_MENU=1u<<3, N3DS_UI_MAP_SIZE=1u<<4
};
void n3dsUiSetTopHudEnabled(int on);
int n3dsUiTopHudEnabled(void);
void n3dsUiPublishPresentation(uint8_t alpha,uint32_t drawn,uint32_t usable,uint32_t blocked);
uint32_t n3dsUiGetPresentation(void);
void n3dsUiSetTouchEnabled(uint32_t actions);
int n3dsUiMapGetSourceRect(int* left,int* top,int* width,int* height);
void n3dsUiSetGpuOwnsBottom(int owns);
uint32_t n3dsUiTouchAction(uint32_t physicalHeld, int inputBlocked);
uint32_t n3dsUiTouchHighlight(void);
int n3dsUiLoadShortcuts(void);
int n3dsUiGetShortcut(unsigned slot,int rect[4],uint32_t* action);
int n3dsUiGetTouchSlot(void);
void n3dsUiPublishItems(uint8_t x, uint8_t y, uint8_t z);
void n3dsUiPublishHeading(int16_t turn, int valid);
uint8_t n3dsUiHeadingStep(void);
void n3dsUiPublishInPlay(int on);
int n3dsUiInPlay(void);
void n3dsUiPublishBindings(uint8_t x, uint8_t y, uint8_t z);
uint32_t n3dsUiGetItems(void);
#ifdef __cplusplus
}
#endif

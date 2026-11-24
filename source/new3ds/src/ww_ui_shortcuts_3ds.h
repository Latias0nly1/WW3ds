#pragma once
#include <stdint.h>
#include "ww_ui_shortcuts_api_3ds.h"
namespace n3ds_ui 
{
    struct Rect 
    {
        int x, y, w, h;
        constexpr bool contains(int px,
int py)const
        {
            return px >= x 
&& py >= y 
&& px < x + w 
&& py < y + h;
            
        }
        
    } ;
    inline constexpr Rect mapContent
    {
        24, 12, 176, 176
    } ;
    struct Target 
    {
        Rect rect;
        uint32_t action;
        
    } ;
    inline constexpr Target targets[] = 
    {
        
        {
            
            {
                220, 3, 96, 48
            }
            , N3DS_UI_ITEM_X
        }
        , 
        {
            
            {
                220, 55, 96, 48
            }
            , N3DS_UI_ITEM_Y
        }
        , 
        
        {
            
            {
                220, 107, 96, 48
            }
            , N3DS_UI_ITEM_Z
        }
        , 
        {
            
            {
                0, 0, 0, 0
            }
            , 0
        }
        , 
        
        {
            
            {
                220, 159, 96, 60
            }
            , N3DS_UI_MAP_SIZE
        }
        
    } ;
    inline constexpr uint32_t allActions = N3DS_UI_ITEM_X|N3DS_UI_ITEM_Y|N3DS_UI_ITEM_Z|N3DS_UI_MAP_SIZE;
    inline uint32_t actionsForItems(uint32_t enabled,
uint32_t items)
    {
        for (unsigned i=0;i<3;++i)if(((items >> (8 * i))&255u) == 255u)enabled&= ~(1u << i);
        return enabled;
        
    }
    inline uint32_t hit(int x,
int y,
uint32_t enabled,
const Target * layout = targets)
    {
        if(x < 0 || y < 0 || x >= 320 || y >= 240)return N3DS_UI_NONE;
        for (unsigned i=0;i<5;++i)
        {
            const auto& t = layout[i];
            if((t.action&enabled) 
&& t.rect.contains(x,
y))return t.action;
            
        }
        return N3DS_UI_NONE;
        
    }
    inline bool insideCaptured(uint32_t action,
int x,
int y)
    {
        constexpr int tolerance = 4;
        for (const auto& t:targets)if(t.action == action)
        {
            const Rect release
            {
                t.rect.x - tolerance, t.rect.y - tolerance, 
                t.rect.w + 2 * tolerance, t.rect.h + 2 * tolerance
            } ;
            return release.contains(x,
y);
            
        }
        return false;
        
    }
    class Touch 
    {
        bool held_ = false, dropped_ = false;
        uint32_t grabbed_ = 0;
        int finger_ = - 1;
        Rect grabbedRect_
        {
            
        } ;
        public:
        void invalidate(uint32_t actions)
        {
            if(grabbed_&actions)
            {
                grabbed_ = 0;
                finger_ = - 1;
                dropped_ = true;
                
            }
            
        }
        int pressedSlot()const
        {
            return grabbed_?finger_: - 1;
            
        }
        uint32_t update(bool down,
int x,
int y,
uint32_t enabled,
bool blocked = false,
const Target * layout = targets)
        {
            if(!down)
            {
                held_ = false;
                dropped_ = false;
                grabbed_ = 0;
                finger_ = - 1;
                return 0;
                
            }
            enabled = blocked?0:enabled&allActions;
            if(!held_)
            {
                held_ = true;
                grabbed_ = 0;
                finger_ = - 1;
                if(x >= 0 
&& y >= 0 
&& x < 320 
&& y < 240)for (unsigned i=0;i<5;++i)
                {
                    const auto& t = layout[i];
                    if((t.action&enabled) 
&& t.rect.contains(x,
y))
                    {
                        grabbed_ = t.action;
                        finger_ = int(i);
                        grabbedRect_ = t.rect;
                        break;
                        
                    }
                    
                }
                dropped_ = grabbed_ == 0;
                
            }
            if(dropped_)return 0;
            const Rect release
            {
                grabbedRect_.x - 4, grabbedRect_.y - 4, grabbedRect_.w + 8, grabbedRect_.h + 8
            } ;
            const auto& current = layout[finger_];
            if(!(enabled&grabbed_) || current.action != grabbed_ || 
            current.rect.x != grabbedRect_.x || current.rect.y != grabbedRect_.y || 
            current.rect.w != grabbedRect_.w || current.rect.h != grabbedRect_.h || 
            !release.contains(x,
y))
            {
                dropped_ = true;
                grabbed_ = 0;
                finger_ = - 1;
                return 0;
                
            }
            return grabbed_;
            
        }
        
    } ;
    
}

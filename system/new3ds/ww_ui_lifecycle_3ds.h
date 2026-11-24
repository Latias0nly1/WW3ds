#pragma once
#include <stdint.h>
namespace n3ds_ui 
{
    enum UiBlock : uint32_t 
    {
        Cinematic = 1, Pause = 2, Transition = 4, Unavailable = 8
    } ;
    struct Presentation 
    {
        uint32_t packed = 0;
        uint8_t alpha() const 
        {
            return uint8_t(packed);
            
        }
        uint32_t drawn() const 
        {
            return (packed >> 8)&31u;
            
        }
        uint32_t usable() const 
        {
            return (packed >> 13)&31u;
            
        }
        uint32_t blocked() const 
        {
            return (packed >> 18)&15u;
            
        }
        
    } ;
    inline Presentation presentation(uint8_t alpha,
uint32_t drawn,
uint32_t usable,
uint32_t blocked)
    {
        blocked&= 15u;
        if(blocked)alpha = 0;
        drawn = alpha?drawn&31u:0;
        usable = alpha == 255 
&& !blocked?usable&drawn&31u:0;
        return 
        {
            uint32_t(alpha)|(drawn << 8)|(usable << 13)|(blocked << 18)
        } ;
        
    }
    
}

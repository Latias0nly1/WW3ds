#pragma once
#include "ww_ui_shortcuts_3ds.h"
#include <cstdio>
#include <cstring>
#include <cctype>
namespace n3ds_ui 
{
    struct ShortcutConfig 
    {
        Target bindings[5];
        ShortcutConfig()
        {
            for (unsigned i=0;i<5;++i)bindings[i] = targets[i];
            
        }
        static bool overlaps(Rect a,
Rect b)
        {
            return a.x < b.x + b.w 
&& b.x < a.x + a.w 
&& a.y < b.y + b.h 
&& b.y < a.y + a.h;
            
        }
        bool valid()const
        {
            constexpr Rect mapFrame
            {
                12, 0, 204, 200
            } ;
            for (unsigned i=0;i<5;++i)
            {
                const auto& s = bindings[i];
                const auto& r = s.rect;
                const auto& fixed = targets[i].rect;
                if(r.x != fixed.x || r.y != fixed.y || r.w != fixed.w || r.h != fixed.h)return false;
                if(!(r.w == 0 
&& r.h == 0 
&& s.action == 0) 
&& (r.x < 0 || r.y < 0 || r.w < 48 || r.h < 28 || r.w > 320 || r.h > 240 || r.x > 320 - r.w || r.y > 240 - r.h))return false;
                if(s.action 
&& (s.action&~allActions || (s.action&(s.action - 1))))return false;
                if(i >= 3 
&& s.action != targets[i].action)return false;
                if(i < 3 
&& (s.action&(N3DS_UI_MENU|N3DS_UI_MAP_SIZE)))return false;
                if(!s.action)continue;
                if(overlaps(r,
mapFrame))return false;
                for (unsigned j=0;j<i;++j)if(bindings[j].action 
&& overlaps(r,
bindings[j].rect))return false;
                
            }
            return true;
            
        }
        static char * trim(char * p)
        {
            while( * p 
&& std::isspace(static_cast < unsigned char > ( * p)))++p;
            char * e = p + std::strlen(p);
            while(e > p 
&& std::isspace(static_cast < unsigned char > (e[ - 1])))--e;
             * e = 0;
            return p;
            
        }
        bool parseLine(char * line)
        {
            if(char * comment = std::strchr(line,
'#')) * comment = 0;
            char * key = trim(line);
            if(! * key)return true;
            char * equal = std::strchr(key,
'=');
            if(!equal || std::strchr(equal + 1,
'='))return false;
             * equal = 0;
            key = trim(key);
            char * value = trim(equal + 1);
            for (char* p=key;*p;++p) * p = std::tolower(static_cast < unsigned char > ( * p));
            if(std::strlen(key) < 6 || std::strncmp(key,
"slot",
4) || key[4] < '1' || key[4] > '5' || key[5] != '.')return false;
            const unsigned index = unsigned(key[4] - '1');
            if(!std::strcmp(key + 6,
"action"))
            {
                for (char* p=value;*p;++p) * p = std::toupper(static_cast < unsigned char > ( * p));
                struct Action 
                {
                    const char * name;
                    uint32_t bits;
                    
                } ;
                static constexpr Action names[] = 
                {
                    
                    {
                        "NONE", 0
                    }
                    , 
                    {
                        "X", N3DS_UI_ITEM_X
                    }
                    , 
                    {
                        "Y", N3DS_UI_ITEM_Y
                    }
                    , 
                    {
                        "Z", N3DS_UI_ITEM_Z
                    }
                    , 
                    {
                        "MENU", N3DS_UI_MENU
                    }
                    , 
                    {
                        "MAP_SIZE", N3DS_UI_MAP_SIZE
                    }
                    
                } ;
                for (const auto& a:names)if(!std::strcmp(value,
a.name))
                {
                    if(index >= 3 
&& a.bits != targets[index].action)return false;
                    if(index < 3 
&& (a.bits&(N3DS_UI_MENU|N3DS_UI_MAP_SIZE)))return false;
                    bindings[index].action = a.bits;
                    return true;
                    
                }
                return false;
                
            }
            return false;
            
        }
        int read(FILE * f)
        {
            if(!f)return - 1;
            ShortcutConfig candidate;
            char line[256];
            unsigned number = 0;
            while(std::fgets(line,
sizeof line,
f))
            {
                ++number;
                if(number > 128)return int(number);
                if(!std::strchr(line,
'\n') 
&& !std::feof(f))
                {
                    const int ch = std::fgetc(f);
                    if(ch != EOF)return int(number);
                    
                }
                if(!candidate.parseLine(line))return int(number);
                
            }
            if(std::ferror(f))return int(number + 1);
            if(!candidate.valid())return - 2;
             * this = candidate;
            return 0;
            
        }
        int load(const char * path)
        {
            FILE * f = std::fopen(path,
"r");
            if(!f)return - 1;
            const int result = read(f);
            std::fclose(f);
            return result;
            
        }
        
    } ;
    
}

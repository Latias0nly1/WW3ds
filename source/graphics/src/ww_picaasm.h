#pragma once
#include <map>
#include <string>
#include <vector>
namespace t2p 
{
    struct PVec 
    {
        float v[4] = 
        {
            0, 0, 0, 0
        } ;
        
    } ;
    struct PicaOperand 
    {
        char file = 'r';
        int reg = 0;
        int idx = 0;
        bool neg = false;
        int swz[4] = 
        {
            0, 1, 2, 3
        } ;
        
    } ;
    struct PicaIns 
    {
        std::string op;
        PicaOperand dst, src[3];
        bool dstMask[4] = 
        {
            true, true, true, true
        } ;
        int nsrc = 0;
        int line = 0;
        
    } ;
    struct PicaOutDecl 
    {
        std::string prop;
        int reg = 0;
        int mask = 0xF;
        
    } ;
    struct PicaProgram2 
    {
        std::vector < PicaIns > ins;
        std::map < std::string, std::pair < int, int >> uniform;
        std::map < int, PVec > constants;
        std::map < std::string, int > inputs;
        std::vector < PicaOutDecl > outputs;
        int fvecRegs = 0;
        int constRegs = 0;
        int opdescs = 0, madOpdescs = 0;
        
    } ;
    bool parsePica(const std::string& text,
PicaProgram2& out,
std::string& err);
    bool runPica(const PicaProgram2&,
const std::map < std::string,
std::vector < PVec >> & uniforms,
const PVec in[16],
    std::map < std::string,
PVec > & out,
std::string& err,
bool f24 = false);
    float roundF24(float);
    
}

#include "ww_approx.h"
#include "ww_bundle.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
namespace t2p 
{
    namespace 
    {
        const char * leafName(Leaf l) 
        {
            static const char * n[] = 
            {
                "CPREV", "APREV", "REG0.rgb", "REG0.a", "REG1.rgb", "REG1.a", "REG2.rgb", "REG2.a", "TEXC", "TEXA", "RASC", "RASA", "ONE", "HALF", "ZERO", "KONST"
            } ;
            return l < 16 ? n[l] : "internal";
            
        }
        float clamp01(float x) 
        {
            return x < 0 ? 0 : x > 1 ? 1 : x;
            
        }
        struct Op 
        {
            enum Kind 
            {
                Zero, One, Pass, Erase, DropTex, DropReg 
            }
            kind = Zero;
            int stage = 0;
            int ch = 0;
            int slot = 0;
            Leaf was = L_ZERO;
            int texmap = - 1, texcoord = - 1;
            int reg = 0;
            
        } ;
        TevChan passthrough(int ch) 
        {
            TevChan p;
            p.in[3] = ch == 0 ? L_CPREV : L_APREV;
            return p;
            
        }
        bool isPassthrough(const TevChan& c,
int ch) 
        {
            return c.out == 0 
&& c.cmp == 0 
&& !c.sub 
&& c.bias == 0 
&& c.scale == 1 
&& c.in[0] == L_ZERO 
&& c.in[1] == L_ZERO 
&& c.in[2] == L_ZERO 
&& c.in[3] == (ch == 0 ? L_CPREV : L_APREV);
            
        }
        struct Applied 
        {
            TevMaterial m;
            std::vector < int > orig;
            
        } ;
        Applied applyOps(const TevMaterial& original,
const std::vector < Op > & ops) 
        {
            TevMaterial m = original;
            std::vector < bool > erase(m.stages.size(),
false);
            for (const Op& op : ops) 
            {
                if (op.kind == Op::DropTex || op.kind == Op::DropReg) 
                {
                    for (TevStage& st : m.stages) 
                    {
                        const bool texHit = op.kind == Op::DropTex 
&& st.texmap == op.texmap 
&& st.texcoord == op.texcoord;
                        for (TevChan* c : {&st.c,
&st.a})
                        for (Leaf& l : c->in) 
                        {
                            if (texHit 
&& (l == L_TEXC || l == L_TEXA)) l = L_ZERO;
                            if (op.kind == Op::DropReg 
&& (l == Leaf(L_C0 + 2 * op.reg) || l == Leaf(L_A0 + 2 * op.reg))) l = L_ZERO;
                            
                        }
                        if (texHit) 
                        {
                            st.texmap = - 1;
                            st.texcoord = - 1;
                            
                        }
                        if (st.a.cmp == 1) setAlphaR8Operands(st);
                        
                    }
                    continue;
                    
                }
                TevStage& st = m.stages[size_t(op.stage)];
                TevChan& ch = op.ch == 0 ? st.c : st.a;
                switch (op.kind) 
                {
                    case Op::Zero: ch.in[op.slot] = L_ZERO;
                    break;
                    case Op::One: ch.in[op.slot] = L_ONE;
                    break;
                    case Op::Pass: ch = passthrough(op.ch);
                    break;
                    case Op::Erase: erase[size_t(op.stage)] = true;
                    break;
                    case Op::DropTex: case Op::DropReg: break;
                    
                }
                if (op.ch == 0 
&& op.kind != Op::Erase 
&& st.a.cmp == 1) setAlphaR8Operands(st);
                
            }
            Applied r;
            for (size_t i = 0; i < m.stages.size(); i++)
            if (!erase[i]) 
            {
                r.m.stages.push_back(m.stages[i]);
                r.orig.push_back(int(i));
                
            }
            return r;
            
        }
        struct Reference 
        {
            std::vector < Inputs > in;
            std::vector < Rgba > out;
            Reference(const TevMaterial& original,
uint64_t seed,
int n) 
            {
                Rng rng(seed);
                for (int i = 0; i < n; i++) 
                {
                    in.push_back(rng.inputs());
                    out.push_back(evalTev(original,
in.back()));
                    
                }
                
            }
            MatErr error(const TevMaterial& m) const 
            {
                MatErr e;
                double sum = 0;
                for (size_t i = 0; i < in.size(); i++) 
                {
                    const Rgba b = evalTev(m,
in[i]);
                    for (int k = 0; k < 4; k++) 
                    {
                        const double d = std::fabs(double(snap8(clamp01(out[i].v[k]))) - double(snap8(clamp01(b.v[k])))) * 255.0;
                        e.max255 = std::max(e.max255,
d);
                        sum += d;
                        
                    }
                    
                }
                e.mean255 = in.empty() ? 0 : sum / (4.0 * double(in.size()));
                return e;
                
            }
            
        } ;
        int trouble(const PicaDraw& r) 
        {
            int t = int(r.blockers.size());
            for (const StageNote& n : r.tev.notes)
            if (n.color == Fidelity::Unsupported || n.alpha == Fidelity::Unsupported) t++;
            return t;
            
        }
        std::string describe(const Op& op,
const TevMaterial& original) 
        {
            char b[160];
            const TevStage& st = original.stages[size_t(op.stage)];
            const char * chn = op.ch == 0 ? "colour" : "alpha";
            switch (op.kind) 
            {
                case Op::Zero: std::snprintf(b,
sizeof b,
"stage %d %s: input %c (%s) set to zero",
op.stage,
chn,
"abcd"[op.slot],
leafName(op.was));
                break;
                case Op::One: std::snprintf(b,
sizeof b,
"stage %d %s: input %c (%s) set to one",
op.stage,
chn,
"abcd"[op.slot],
leafName(op.was));
                break;
                case Op::Pass: std::snprintf(b,
sizeof b,
"stage %d %s: replaced by Previous (texmap %d)",
op.stage,
chn,
st.texmap);
                break;
                case Op::Erase: std::snprintf(b,
sizeof b,
"stage %d removed",
op.stage);
                break;
                case Op::DropTex: std::snprintf(b,
sizeof b,
"every read of texture (texmap %d, texcoord %d) set to zero",
op.texmap,
op.texcoord);
                break;
                case Op::DropReg: std::snprintf(b,
sizeof b,
"every read of TEV register %d set to zero",
op.reg);
                break;
                
            }
            return b;
            
        }
        
    }
    MatErr materialError(const TevMaterial& original,
const TevMaterial& degraded,
uint64_t seed,
int n) 
    {
        return Reference(original,
seed,
n).error(degraded);
        
    }
    bool compileApprox(const GxDrawState& g,
const Options& opt,
const PicaDraw& failed,
PicaDraw& out,
std::string& why) 
    {
        if (!g.importBlockers.empty()) 
        {
            why = "the draw state has parts the model cannot represent (" + g.importBlockers[0] + ")";
            return false;
            
        }
        if (g.tev.stages.empty()) 
        {
            why = "no TEV stages";
            return false;
            
        }
        Options o = opt;
        o.approximate = false;
        const MaterialBundle * look = bundleLookup();
        const std::string key = (look || bundleRecorder()) ? approxKey(g.tev) : std::string();
        if (look) 
        {
            if (const ApproxEntry * e = look->findApprox(key)) 
            {
                PicaDraw r;
                if (e->relaxed) 
                {
                    Options rel = o;
                    rel.approxLimit = 1e9;
                    r = compileDrawSingle(g,
rel);
                    
                }
                else 
                {
                    GxDrawState g2 = g;
                    g2.tev = e->material;
                    r = compileDrawExact(g2,
o);
                    
                }
                if (r.overall != Fidelity::Unsupported) 
                {
                    out = r;
                    out.blockers.clear();
                    out.overall = Fidelity::Approx;
                    out.notes.insert(out.notes.begin(),
e->note);
                    bundleCountApproxHit();
                    return true;
                    
                }
                
            }
            
        }
        const Reference ref(g.tev,
0xA9910C0FFEEULL,
192);
        int compiles = 0;
        const int kBudget = 600, kTries = std::min(10,
opt.approxEffort);
        auto compile = [&](const TevMaterial& m,
PicaDraw& r) 
        {
            GxDrawState g2 = g;
            g2.tev = m;
            r = compileDrawExact(g2,
o);
            compiles++;
            return r.overall != Fidelity::Unsupported;
            
        } ;
        PicaDraw relaxedDraw;
        bool relaxedOk = false;
        MatErr relaxedErr
        {
            1e9, 1e9
        } ;
        if (opt.approxEffort > 0) 
        {
            Options rel = o;
            rel.approxLimit = 1e9;
            PicaDraw r = compileDrawSingle(g,
rel);
            compiles++;
            if (r.overall != Fidelity::Unsupported) 
            {
                relaxedOk = true;
                relaxedDraw = r;
                relaxedErr = 
                {
                    r.tev.verify.meanErr255, r.tev.verify.maxErr255
                } ;
                
            }
            
        }
        std::vector < Op > ops;
        PicaDraw cur = failed, good;
        int curTrouble = trouble(failed);
        bool found = false;
        for (int round = 0; round < 80 
&& !found 
&& compiles < kBudget 
&& kTries > 0; round++) 
        {
            const Applied A = applyOps(g.tev,
ops);
            std::set < int > allowed;
            for (const StageNote& n : cur.tev.notes)
            if (n.gcStage >= 0 
&& n.gcStage < int(A.orig.size()) 
&& (n.color == Fidelity::Unsupported || n.alpha == Fidelity::Unsupported)) allowed.insert(A.orig[size_t(n.gcStage)]);
            if (allowed.empty()) allowed.insert(A.orig.begin(),
A.orig.end());
            std::string why;
            for (const std::string& b : cur.blockers) why += b + "\n";
            for (const StageNote& n : cur.tev.notes) why += n.text + "\n";
            const bool wantTex = why.find("texture units") != std::string::npos || why.find("distinct (texmap") != std::string::npos;
            const bool wantStage = why.find("PICA stages, hardware has") != std::string::npos;
            const bool wantReg = why.find("register") != std::string::npos;
            struct Cand 
            {
                Op op;
                double err;
                int prio;
                
            } ;
            std::vector < Cand > cands;
            for (size_t i = 0; i < A.m.stages.size(); i++) 
            {
                const int os = A.orig[i];
                if (!allowed.count(os)) continue;
                const TevStage& st = A.m.stages[i];
                auto add = [&](Op op) 
                {
                    std::vector < Op > t = ops;
                    t.push_back(op);
                    int prio = 2;
                    if (op.kind == Op::Erase 
&& (wantStage || wantReg)) prio = 1;
                    cands.push_back(
                    {
                        op,
ref.error(applyOps(g.tev,
t).m).mean255,
prio
                    }
                    );
                    
                } ;
                
                {
                    Op e;
                    e.kind = Op::Erase;
                    e.stage = os;
                    add(e);
                    
                }
                for (int ch = 0; ch < 2; ch++) 
                {
                    const TevChan& c = ch == 0 ? st.c : st.a;
                    if (ch == 0 
&& st.a.cmp == 1) 
                    {
                        
                    }
                    const bool guardColour = ch == 0 
&& st.a.cmp == 1;
                    if (c.out == 0 
&& !isPassthrough(c,
ch) 
&& !guardColour) 
                    {
                        Op p;
                        p.kind = Op::Pass;
                        p.stage = os;
                        p.ch = ch;
                        add(p);
                        
                    }
                    if (c.cmp != 0) continue;
                    for (int s = 0; s < 4; s++) 
                    {
                        if (guardColour 
&& s < 2) continue;
                        if (c.in[s] == L_ZERO) continue;
                        Op z;
                        z.kind = Op::Zero;
                        z.stage = os;
                        z.ch = ch;
                        z.slot = s;
                        z.was = c.in[s];
                        add(z);
                        if (ch == 0 
&& s == 2 
&& c.in[s] != L_ONE) 
                        {
                            Op one = z;
                            one.kind = Op::One;
                            add(one);
                            
                        }
                        
                    }
                    
                }
                
            }
            
            {
                std::set < std::pair < int, int >> pairs;
                bool regRead[3] = 
                {
                    false, false, false
                } ;
                for (const TevStage& st : A.m.stages)
                for (const TevChan* c : {&st.c,
&st.a})
                for (Leaf l : c->in) 
                {
                    if ((l == L_TEXC || l == L_TEXA) 
&& st.texmap >= 0) pairs.insert(
                    {
                        st.texmap,
st.texcoord
                    }
                    );
                    for (int r = 0; r < 3; r++) if (l == Leaf(L_C0 + 2 * r) || l == Leaf(L_A0 + 2 * r)) regRead[r] = true;
                    
                }
                auto addGlobal = [&](Op op,
int prio) 
                {
                    std::vector < Op > t = ops;
                    t.push_back(op);
                    cands.push_back(
                    {
                        op,
ref.error(applyOps(g.tev,
t).m).mean255,
prio
                    }
                    );
                    
                } ;
                if (pairs.size() > 1) for (const auto& pr : pairs) 
                {
                    Op o2;
                    o2.kind = Op::DropTex;
                    o2.texmap = pr.first;
                    o2.texcoord = pr.second;
                    addGlobal(o2,
wantTex ? 0 : 3);
                    
                }
                for (int r = 0; r < 3; r++) if (regRead[r]) 
                {
                    Op o2;
                    o2.kind = Op::DropReg;
                    o2.reg = r;
                    addGlobal(o2,
wantReg ? 0 : 3);
                    
                }
                
            }
            if (cands.empty()) break;
            std::stable_sort(cands.begin(),
cands.end(),
[](const Cand& a,
const Cand& b) 
            {
                return a.prio != b.prio ? a.prio < b.prio : a.err < b.err; 
            }
            );
            int bestTrouble = 1 << 30, bestIdx = - 1;
            PicaDraw bestFail;
            for (size_t k = 0; k < cands.size() 
&& int(k) < kTries 
&& compiles < kBudget; k++) 
            {
                std::vector < Op > t = ops;
                t.push_back(cands[k].op);
                PicaDraw r;
                if (compile(applyOps(g.tev,
t).m,
r)) 
                {
                    ops = t;
                    good = r;
                    found = true;
                    break;
                    
                }
                const int tr = trouble(r);
                if (tr < bestTrouble) 
                {
                    bestTrouble = tr;
                    bestIdx = int(k);
                    bestFail = r;
                    
                }
                
            }
            if (found) break;
            if (bestIdx < 0) break;
            if (bestTrouble >= curTrouble) 
            {
                bestIdx = 0;
                std::vector < Op > t = ops;
                t.push_back(cands[0].op);
                compile(applyOps(g.tev,
t).m,
bestFail);
                bestTrouble = trouble(bestFail);
                
            }
            ops.push_back(cands[size_t(bestIdx)].op);
            cur = bestFail;
            curTrouble = bestTrouble;
            
        }
        std::vector < Op > greedyOps = ops;
        const bool greedyOk = found 
&& kTries > 0;
        PicaDraw greedyDraw = good;
        if (greedyOk) 
        {
            for (int i = int(greedyOps.size()) - 1; i >= 0 
&& compiles < kBudget; i--) 
            {
                std::vector < Op > t = greedyOps;
                t.erase(t.begin() + i);
                PicaDraw r;
                if (compile(applyOps(g.tev,
t).m,
r)) 
                {
                    greedyOps = t;
                    greedyDraw = r;
                    
                }
                
            }
            
        }
        const MatErr greedyErr = greedyOk ? ref.error(applyOps(g.tev,
greedyOps).m) : MatErr
        {
            1e9, 1e9
        } ;
        TevMaterial replacement;
        bool haveTex = false;
        
        {
            TevStage s;
            for (const TevStage& st : g.tev.stages) 
            {
                bool rt = false;
                for (const TevChan* ch : {&st.c,
&st.a}) for (Leaf l : ch->in) if (l == L_TEXC || l == L_TEXA) rt = true;
                if (rt 
&& st.texmap >= 0 
&& st.texcoord >= 0) 
                {
                    s = st;
                    haveTex = true;
                    break;
                    
                }
                
            }
            s.c = TevChan();
            s.a = TevChan();
            s.kc = KSel();
            s.ka = KSel();
            if (haveTex) 
            {
                s.c.in[1] = L_TEXC;
                s.c.in[2] = L_RASC;
                s.a.in[1] = L_TEXA;
                s.a.in[2] = L_RASA;
                
            }
            else 
            {
                s.c.in[3] = L_RASC;
                s.a.in[3] = L_RASA;
                s.texmap = - 1;
                s.texcoord = - 1;
                
            }
            replacement.stages.push_back(s);
            
        }
        PicaDraw resortDraw;
        const bool resortOk = compile(replacement,
resortDraw);
        const MatErr resortErr = resortOk ? ref.error(replacement) : MatErr
        {
            1e9, 1e9
        } ;
        if (!greedyOk 
&& !resortOk 
&& !relaxedOk) 
        {
            why = "no degradation of the material could be drawn either";
            return false;
            
        }
        enum 
        {
            Greedy, Resort, Relaxed 
        }
        pick = Greedy;
        double best = 1e18;
        if (greedyOk 
&& greedyErr.mean255 < best) 
        {
            best = greedyErr.mean255;
            pick = Greedy;
            
        }
        if (resortOk 
&& resortErr.mean255 < best) 
        {
            best = resortErr.mean255;
            pick = Resort;
            
        }
        if (relaxedOk 
&& relaxedErr.mean255 < best) 
        {
            best = relaxedErr.mean255;
            pick = Relaxed;
            
        }
        std::string lastResort;
        MatErr err = greedyErr;
        if (pick == Resort) 
        {
            lastResort = haveTex ? "the material was replaced by its first texture times the vertex colour" : "the material was replaced by the vertex colour";
            err = resortErr;
            good = resortDraw;
            ops.clear();
            
        }
        else if (pick == Relaxed) 
        {
            lastResort = "the translator's best approximations were accepted (error limit lifted)";
            err = relaxedErr;
            good = relaxedDraw;
            ops.clear();
            
        }
        else 
        {
            good = greedyDraw;
            ops = greedyOps;
            
        }
        out = good;
        out.blockers.clear();
        out.overall = Fidelity::Approx;
        char b[200];
        std::string why0 = failed.blockers.empty() ? std::string("the material does not fit") : failed.blockers[0];
        if (why0.size() > 160) why0 = why0.substr(0,
160) + "...";
        std::string note = "approximate rung: the material cannot be drawn exactly, not even in two passes (" + why0 + ")";
        if (!lastResort.empty()) 
        {
            note += "; " + lastResort;
            
        } else 
        {
            note += "; degraded: ";
            for (size_t i = 0; i < ops.size(); i++) note += (i ? "; " : "") + describe(ops[i],
g.tev);
            
        }
        std::snprintf(b,
sizeof b,
" -- difference from the original material on random inputs: mean %.1f/255, worst %.0f/255",
err.mean255,
err.max255);
        note += b;
        out.notes.insert(out.notes.begin(),
note);
        if (MaterialBundle * rec = bundleRecorder()) 
        {
            ApproxEntry e;
            e.relaxed = pick == Relaxed;
            e.material = pick == Greedy ? applyOps(g.tev,
ops).m : (pick == Resort ? replacement : g.tev);
            e.note = note;
            rec->addApprox(key,
e);
            
        }
        return true;
        
    }
    
}

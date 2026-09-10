// Exact global 13-active linear-trail optimizer for the full Option 1 rotor layer.
//
// Scope: three rounds, full 32-cell / 96-shear butterfly, rotor schedule,
// free nonzero endpoint masks, AES S-box. The exact global three-round
// activity minimum (13) is established separately in GLOBAL_THREE_ROUND_ACTIVITY.md.
// This checker closes the value-level question at that minimum.
//
// It enumerates every structurally admissible 13-active split after exact
// branch-equality and one-active endpoint pruning. For each split it joins
// exact GF(2) relations on the shared middle-byte support and evaluates every
// compatible AES LAT transition. No beam width or heuristic cutoff is used.
//
// Option 1 branch only; not part of the structural-limits paper under review.

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using U128 = __uint128_t;
using M8 = std::array<std::uint8_t, 8>;
using State = std::array<std::uint8_t, 16>;

static const M8 BASE = {0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82};
static std::array<M8,4> MF;
static std::array<std::vector<std::uint16_t>,17> COMB;
static std::array<U128,1u<<16> FORB;
static std::array<std::uint8_t,256> SBOX;
static double LAT_COST[256][256];

struct Step { int t,s,k; };
struct Rel { State a,b; };
struct Split { int a,b,c; };
struct Best {
    bool found=false;
    double cost=std::numeric_limits<double>::infinity();
    double middle_cost=0;
    Split split{};
    std::uint16_t middle_support=0;
    State z1{},x2{},z2{},x3{};
    std::uint64_t relation_pairs=0;
};

static int par(std::uint8_t x){ return __builtin_popcount((unsigned)x)&1; }
static std::uint8_t am(const M8&m,std::uint8_t x){
    std::uint8_t y=0; for(int i=0;i<8;i++) y|=std::uint8_t(par(m[i]&x)<<(7-i)); return y;
}
static M8 rot(const M8&m){
    int a[8][8]={},b[8][8]={};
    for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];
    M8 o{}; for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c); return o;
}
static std::vector<std::pair<int,int>> pairs(int mask){
    std::vector<std::pair<int,int>>v; for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask}); return v;
}
static void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
static std::vector<Step> steps(int round){
    std::vector<Step>v; int masks[4]={1,2,4,8};
    for(int st=0;st<4;st++){
        auto ps=pairs(masks[st]);
        for(int pi=0;pi<8;pi++){int k=(round+st+pi)&3;cell(v,ps[pi].first,ps[pi].second,k);}
    }
    return v;
}
static State layer(State x,int round,bool inv=false){
    auto v=steps(round);
    if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}
    else{for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);}
    return x;
}
static U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;}
static State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=std::uint8_t(v&255);v>>=8;}return x;}
static std::array<U128,128> cols(int round,bool inv=false){
    std::array<U128,128>C{};
    for(int j=0;j<128;j++){State x{};x[j/8]=std::uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x,round,inv));}
    return C;
}
static std::array<U128,128> trans(const std::array<U128,128>&C){
    std::array<U128,128>T{};
    for(int j=0;j<128;j++){
        U128 v=0; for(int r=0;r<128;r++) if((C[r]>>(127-j))&1) v|=(U128)1<<(127-r); T[j]=v;
    }
    return T;
}
static State applyC(const std::array<U128,128>&C,const State&x){
    U128 y=0; for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit]; return unpack(y);
}
static U128 bmask(int b){return(U128)255<<(8*(15-b));}
static int top(U128 x){
    std::uint64_t h=(std::uint64_t)(x>>64); if(h)return 64+63-__builtin_clzll(h);
    std::uint64_t l=(std::uint64_t)x; return l?63-__builtin_clzll(l):-1;
}
static int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;}
static std::uint16_t supp(const State&x){std::uint16_t s=0;for(int b=0;b<16;b++)if(x[b])s|=std::uint16_t(1u<<b);return s;}
static std::string hx(const State&x){static const char*d="0123456789abcdef";std::string s;for(auto b:x){s+=d[b>>4];s+=d[b&15];}return s;}

static std::vector<std::uint64_t> nullbasis(const std::vector<U128>&vs){
    U128 bas[128]={}; std::uint64_t cf[128]={}; std::vector<std::uint64_t>N;
    if(vs.size()>63) throw std::runtime_error("input support exceeds 63-bit coefficient encoding");
    for(int j=0;j<(int)vs.size();j++){
        U128 x=vs[j]; std::uint64_t c=1ULL<<j;
        while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}
        if(!x)N.push_back(c);
    }
    return N;
}
static State coeffstate(std::uint16_t I,std::uint64_t c){
    State x{};int p=0;
    for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=std::uint8_t(1u<<(7-bit));p++;}
    return x;
}
static bool independent(const std::vector<U128>&vs){
    U128 bas[128]={};
    for(auto x:vs){while(x){int b=top(x);if(bas[b])x^=bas[b];else{bas[b]=x;break;}}if(!x)return false;}
    return true;
}
static bool envelope_possible(const std::array<U128,128>&C,int input_weight,std::uint16_t J){
    U128 fm=FORB[J];
    for(auto I:COMB[input_weight]){
        std::vector<U128>p; p.reserve(8*input_weight);
        for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);
        if(!independent(p))return true;
    }
    return false;
}
static std::vector<Rel> to_output(const std::array<U128,128>&C,int input_weight,std::uint16_t J){
    int output_weight=__builtin_popcount((unsigned)J); std::vector<Rel>R; U128 fm=FORB[J];
    for(auto I:COMB[input_weight]){
        std::vector<U128>p; p.reserve(8*input_weight);
        for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);
        auto nb=nullbasis(p); if(nb.empty())continue;
        if(nb.size()>24) throw std::runtime_error("unexpected nullity >24; exhaustive enumeration intentionally aborts");
        std::uint64_t total=1ULL<<nb.size();
        for(std::uint64_t q=1;q<total;q++){
            std::uint64_t cc=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)cc^=nb[k];
            State a=coeffstate(I,cc); if(wt(a)!=input_weight)continue;
            State b=applyC(C,a); if(wt(b)==output_weight&&supp(b)==J)R.push_back({a,b});
        }
    }
    return R;
}

static std::uint8_t gm(std::uint8_t a,std::uint8_t b){std::uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;}
static std::uint8_t gp(std::uint8_t a,int e){std::uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;}
static std::uint8_t rol(std::uint8_t x,int n){return std::uint8_t((x<<n)|(x>>(8-n)));}
static std::uint8_t sb(std::uint8_t x){std::uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
static void init(){
    MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);
    for(int x=0;x<256;x++)SBOX[x]=sb((std::uint8_t)x);
    for(std::uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((std::uint16_t)m);
    U128 all=~(U128)0;for(std::uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}
    for(int a=0;a<256;a++)for(int b=0;b<256;b++)LAT_COST[a][b]=std::numeric_limits<double>::infinity();
    for(int a=1;a<256;a++)for(int b=1;b<256;b++){
        int w=0;for(int x=0;x<256;x++)w+=(par((std::uint8_t)(a&x))==par((std::uint8_t)(b&SBOX[x])))?1:-1;
        if(w)LAT_COST[a][b]=8.0-std::log2((double)std::abs(w));
    }
}

static Best optimize_split(const std::array<U128,128>&T0,const std::array<U128,128>&T1inv,Split sp){
    Best best; best.split=sp;
    for(auto K:COMB[sp.b]){
        if(!envelope_possible(T0,sp.a,K))continue;
        if(!envelope_possible(T1inv,sp.c,K))continue;
        auto left=to_output(T0,sp.a,K); if(left.empty())continue;
        auto right_rev=to_output(T1inv,sp.c,K); if(right_rev.empty())continue;
        for(const auto&r0:left)for(const auto&rr:right_rev){
            best.relation_pairs++;
            const State&x2=r0.b; const State&z2=rr.b;
            double mc=0; bool ok=true;
            for(int b=0;b<16;b++)if(x2[b]){
                double q=LAT_COST[x2[b]][z2[b]]; if(!std::isfinite(q)){ok=false;break;} mc+=q;
            }
            if(!ok)continue;
            double total=3.0*(sp.a+sp.c)+mc;
            if(total<best.cost){best.found=true;best.cost=total;best.middle_cost=mc;best.middle_support=K;best.z1=r0.a;best.x2=x2;best.z2=z2;best.x3=rr.a;}
        }
    }
    return best;
}

int main(int argc,char**argv){
    init(); int phase=argc>1?std::stoi(argv[1]):0; if(phase<0||phase>3){std::cerr<<"phase must be 0..3\n";return 2;}
    auto T0=trans(cols(phase,true));
    auto T1inv=trans(cols(phase+1,false));

    const std::array<Split,11> candidates = {{{2,7,4},{2,8,3},{2,9,2},{3,6,4},{3,7,3},{3,8,2},{4,4,5},{4,5,4},{4,6,3},{4,7,2},{5,4,4}}};
    Best global; int support_feasible_splits=0;
    for(auto sp:candidates){
        Best b=optimize_split(T0,T1inv,sp);
        std::cout<<"split="<<sp.a<<"->"<<sp.b<<"->"<<sp.c<<" ";
        if(!b.found){std::cout<<"no_value_trail relation_pairs="<<b.relation_pairs<<"\n";continue;}
        support_feasible_splits++;
        std::cout<<"cost_bits="<<std::fixed<<std::setprecision(6)<<b.cost<<" middle_cost="<<b.middle_cost<<" relation_pairs="<<b.relation_pairs<<"\n";
        if(!global.found||b.cost<global.cost)global=b;
    }
    if(!global.found){std::cerr<<"no 13-active linear trail found\n";return 3;}
    std::cout<<"GLOBAL phase="<<phase<<" active=13 split="<<global.split.a<<"->"<<global.split.b<<"->"<<global.split.c
             <<" cost_bits="<<std::fixed<<std::setprecision(6)<<global.cost<<" support_feasible_splits="<<support_feasible_splits<<"\n"
             <<"middle_support=0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<global.middle_support<<std::dec<<"\n"
             <<"z1="<<hx(global.z1)<<"\n"
             <<"x2="<<hx(global.x2)<<"\n"
             <<"z2="<<hx(global.z2)<<"\n"
             <<"x3="<<hx(global.x3)<<"\n";
    return 0;
}

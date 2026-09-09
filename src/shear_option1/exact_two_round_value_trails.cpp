#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82};
static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB;
struct Step{int t,s,k;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;}
uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++) y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c);return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;}
void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(int round,bool rotor,int keep){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<(int)ps.size();pi++){if(st==3 && !((keep>>pi)&1)) continue;int k=rotor?((round+st+pi)&3):0;cell(v,ps[pi].first,ps[pi].second,k);}}return v;}
State apply(State x,int round,bool rotor,int keep,bool inv){auto v=steps(round,rotor,keep);if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}else{for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);}return x;}
U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;}
std::array<U128,128> cols(int round,bool rotor,int keep,bool inv){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(apply(x,round,rotor,keep,inv));}return C;}
U128 bmask(int b){return (U128)0xff << (8*(15-b));}
int top(U128 x){uint64_t hi=(uint64_t)(x>>64);if(hi)return 64+63-__builtin_clzll(hi);uint64_t lo=(uint64_t)x;return lo?63-__builtin_clzll(lo):-1;}
bool indep(const std::vector<U128>&vs){U128 bas[128]={};for(auto x:vs){while(x){int b=top(x);if(bas[b])x^=bas[b];else{bas[b]=x;break;}}if(!x)return false;}return true;}
std::array<U128,128> transpose_cols(const std::array<U128,128>&C){std::array<U128,128>T{};for(int j=0;j<128;j++){U128 v=0;for(int r=0;r<128;r++){if((C[r]>>(127-j))&1) v |= (U128)1<<(127-r);}T[j]=v;}return T;}
bool feasible(const std::array<U128,128>&C,int t,int u,uint16_t &wi,uint16_t &wj){for(auto I:COMB[t]){std::vector<U128>sel;sel.reserve(8*t);for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(C[8*b+bit]);for(auto J:COMB[u]){U128 m=FORB[J];std::vector<U128>p; p.reserve(sel.size());for(auto v:sel)p.push_back(v&m);if(!indep(p)){wi=I;wj=J;return true;}}}return false;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}

uint8_t gmul(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&0x80;a<<=1;if(c)a^=0x1b;b>>=1;}return o;}
uint8_t gpow(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gmul(o,a);a=gmul(a,a);e>>=1;}return o;}
uint8_t rol8(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));}
uint8_t aes_sbox(uint8_t x){uint8_t v=x?gpow(x,254):0;return v^rol8(v,1)^rol8(v,2)^rol8(v,3)^rol8(v,4)^0x63;}
struct Branch {int B=0,t=0,u=0;};
Branch branch_of(const std::array<U128,128>&C,const std::array<U128,128>&CI){
    for(int sum=2;sum<=17;sum++) for(int t=1;t<=sum/2;t++){int u=sum-t;if(u>16)continue;uint16_t I=0,J=0;
        if(feasible(C,t,u,I,J)) return {sum,t,u};
        if(t<u && feasible(CI,t,u,I,J)) return {sum,u,t};
    }
    return {};
}
int main(int argc,char**argv){
    init();
    std::string sched=argc>1?argv[1]:"static"; int round=argc>2?std::stoi(argv[2]):0; int keep=argc>3?std::stoi(argv[3],nullptr,0):0xff;
    if(sched!="static"&&sched!="rotor") return 2; bool rotor=sched=="rotor";
    auto L=cols(round,rotor,keep,false), Linv=cols(round,rotor,keep,true);
    auto T=transpose_cols(Linv), Tinv=transpose_cols(L);
    Branch bd=branch_of(L,Linv), bl=branch_of(T,Tinv);

    std::array<uint8_t,256>S{}; for(int x=0;x<256;x++)S[x]=aes_sbox((uint8_t)x);
    int ddt[256][256]={}; for(int dx=0;dx<256;dx++) for(int x=0;x<256;x++) ddt[dx][S[x]^S[x^dx]]++;
    bool ddt_uniform=true; for(int v=1;v<256;v++){int row=0,col=0;for(int q=1;q<256;q++){row=std::max(row,ddt[v][q]);col=std::max(col,ddt[q][v]);} if(row!=4||col!=4) ddt_uniform=false;}
    bool lat_uniform=true; int maxwalsh=0;
    for(int a=1;a<256;a++) for(int b=1;b<256;b++){int w=0;for(int x=0;x<256;x++) w += (par((uint8_t)(a&x))==par((uint8_t)(b&S[x])))?1:-1;maxwalsh=std::max(maxwalsh,std::abs(w));}
    for(int v=1;v<256;v++){int row=0,col=0;for(int q=1;q<256;q++){int wr=0,wc=0;for(int x=0;x<256;x++){wr+=(par((uint8_t)(v&x))==par((uint8_t)(q&S[x])))?1:-1;wc+=(par((uint8_t)(q&x))==par((uint8_t)(v&S[x])))?1:-1;}row=std::max(row,std::abs(wr));col=std::max(col,std::abs(wc));}if(row!=32||col!=32)lat_uniform=false;}
    std::cout<<"schedule="<<sched<<" round="<<round<<" keep=0x"<<std::hex<<keep<<std::dec<<"\n";
    std::cout<<"differential_branch="<<bd.B<<" split="<<bd.t<<"->"<<bd.u<<"\n";
    std::cout<<"linear_mask_branch="<<bl.B<<" split="<<bl.t<<"->"<<bl.u<<"\n";
    std::cout<<"all_nonzero_DDT_rows_and_columns_attain_4="<<(ddt_uniform?"yes":"no")<<"\n";
    std::cout<<"all_nonzero_LAT_rows_and_columns_attain_abs32="<<(lat_uniform?"yes":"no")<<" max_abs_walsh="<<maxwalsh<<"\n";
    if(ddt_uniform) std::cout<<"exact_two_round_best_differential_characteristic=2^-"<<(6*bd.B)<<"\n";
    if(lat_uniform) std::cout<<"exact_two_round_best_linear_correlation_magnitude=2^-"<<(3*bl.B)<<"\n";
    return 0;
}

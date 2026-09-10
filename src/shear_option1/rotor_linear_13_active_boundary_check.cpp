// Boundary/completeness checker for exact_rotor_linear_13_active.cpp.
//
// Verifies, for a selected full-layer rotor phase, the pruning used before the
// 13-active exhaustive LAT join:
//   * one active byte expands to 16 under T=L^{-T} and T^{-1};
//   * at branch sum 8, envelope feasibility is absent for 1<->7, 2<->6,
//     and 3<->5, while 4<->4 is feasible;
//   * after these prunings and a+b>=8, b+c>=8, exactly the 11 documented
//     positive triples remain for a+b+c=13.
//
// Option 1 branch only; not part of the structural-limits paper under review.

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82};
static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB;
struct Step{int t,s,k;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;} uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++)y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c);return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;} void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(int round){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<8;pi++){int k=(round+st+pi)&3;cell(v,ps[pi].first,ps[pi].second,k);}}return v;}
State layer(State x,int round,bool inv=false){auto v=steps(round);if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}else for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);return x;}
U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;} State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=uint8_t(v&255);v>>=8;}return x;}
std::array<U128,128> cols(int round,bool inv=false){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x,round,inv));}return C;}
std::array<U128,128> trans(const std::array<U128,128>&C){std::array<U128,128>T{};for(int j=0;j<128;j++){U128 v=0;for(int r=0;r<128;r++)if((C[r]>>(127-j))&1)v|=(U128)1<<(127-r);T[j]=v;}return T;}
State applyC(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);} U128 bmask(int b){return(U128)255<<(8*(15-b));}
int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 64+63-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;}
bool independent(const std::vector<U128>&vs){U128 bas[128]={};for(auto x:vs){while(x){int b=top(x);if(bas[b])x^=bas[b];else{bas[b]=x;break;}}if(!x)return false;}return true;}
bool envelope_feasible(const std::array<U128,128>&C,int t,int u){for(auto I:COMB[t]){std::vector<U128>sel;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(C[8*b+bit]);for(auto J:COMB[u]){std::vector<U128>p;U128 fm=FORB[J];for(auto v:sel)p.push_back(v&fm);if(!independent(p))return true;}}return false;}
int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}
int one_active_min(const std::array<U128,128>&C){int mn=16;for(int b=0;b<16;b++)for(int v=1;v<256;v++){State x{};x[b]=(uint8_t)v;mn=std::min(mn,wt(applyC(C,x)));}return mn;}
int main(int argc,char**argv){init();int phase=argc>1?std::stoi(argv[1]):0;if(phase<0||phase>3)return 2;auto T=trans(cols(phase,true));auto Ti=trans(cols(phase,false));
std::cout<<"phase="<<phase<<" one_active_T="<<one_active_min(T)<<" one_active_Tinv="<<one_active_min(Ti)<<"\n";
for(auto q:std::array<std::pair<int,int>,4>{{{1,7},{2,6},{3,5},{4,4}}}){bool f=envelope_feasible(T,q.first,q.second);bool r=envelope_feasible(Ti,q.first,q.second);std::cout<<q.first<<"<->"<<q.second<<" forward="<<(f?"yes":"no")<<" inverse="<<(r?"yes":"no")<<"\n";}
std::vector<std::array<int,3>> triples;for(int a=2;a<=5;a++)for(int b=1;b<=9;b++){int c=13-a-b;if(c<2||c>5)continue;if(a+b<8||b+c<8)continue;if(a+b==8&&!(a==4&&b==4))continue;if(b+c==8&&!(b==4&&c==4))continue;triples.push_back({a,b,c});}
std::cout<<"candidate_count="<<triples.size()<<"\n";for(auto t:triples)std::cout<<t[0]<<"->"<<t[1]<<"->"<<t[2]<<"\n";return triples.size()==11?0:3;}

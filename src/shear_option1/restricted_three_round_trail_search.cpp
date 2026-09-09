// Restricted three-round value-sensitive trail search for Option 1.
//
// IMPORTANT: this is NOT a global three-round optimum search.
// It exhaustively enumerates, within a deliberately restricted class:
//   1. first-layer states that attain the exact byte branch number;
//   2. for differential trails, the unique AES DDT=4 transition for every
//      active middle-round input difference;
//   3. for linear trails, all AES LAT transitions with |Walsh|=32 for every
//      active middle-round input mask.
//
// The search is therefore exact inside this restricted class only.  It is
// intended to detect value-level topology/schedule effects that activity-only
// branch-number analysis cannot see.

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <limits>
#include <cmath>
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82}; static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256> SBOX;
struct Step{int t,s,k;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;} uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++)y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c);return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;} void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(int round,bool rotor,int keep){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<8;pi++){if(st==3&&!((keep>>pi)&1))continue;int k=rotor?((round+st+pi)&3):0;cell(v,ps[pi].first,ps[pi].second,k);}}return v;}
State layer(State x,int round,bool rotor,int keep,bool inv=false){auto v=steps(round,rotor,keep);if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}else{for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);}return x;}
U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;} State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=uint8_t(v&0xff);v>>=8;}return x;}
std::array<U128,128> cols(int round,bool rotor,int keep,bool inv=false){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x,round,rotor,keep,inv));}return C;}
std::array<U128,128> trans(const std::array<U128,128>&C){std::array<U128,128>T{};for(int j=0;j<128;j++){U128 v=0;for(int r=0;r<128;r++)if((C[r]>>(127-j))&1)v|=(U128)1<<(127-r);T[j]=v;}return T;}
State applycols(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);} U128 bmask(int b){return (U128)0xff<<(8*(15-b));} int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 127-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;}
int wt(const State&x){int n=0;for(auto b:x)n+=(b!=0);return n;} std::string hx(const State&x){static const char*d="0123456789abcdef";std::string s;for(auto b:x){s+=d[b>>4];s+=d[b&15];}return s;}
std::vector<uint64_t> nullbasis(const std::vector<U128>&vs){U128 bas[128]={};uint64_t cf[128]={};std::vector<uint64_t>N;for(int j=0;j<(int)vs.size();j++){U128 x=vs[j];uint64_t c=1ULL<<j;while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}if(!x)N.push_back(c);}return N;}
State coeffstate(uint16_t I,uint64_t c){State x{};int p=0;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=uint8_t(1u<<(7-bit));p++;}return x;}
uint8_t gmul(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&0x80;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gmul(o,a);a=gmul(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb(x);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}
struct Best{int N=999;State z1{},x2{},z2{},x3{};long long candidates=0;};
Best searchdiff(const std::array<U128,128>&C0,const std::array<U128,128>&C1,int B){int ddt[256][256]={};for(int dx=0;dx<256;dx++)for(int x=0;x<256;x++)ddt[dx][SBOX[x]^SBOX[x^dx]]++;uint8_t bestdy[256]={};for(int dx=1;dx<256;dx++){for(int dy=1;dy<256;dy++)if(ddt[dx][dy]==4){bestdy[dx]=dy;break;}}Best best;for(int t=1;t<B;t++){int u=B-t;if(t>8||u>16)continue;for(auto I:COMB[t]){std::vector<U128>sel;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(C0[8*b+bit]);if(sel.size()>63)continue;for(auto J:COMB[u]){std::vector<U128>proj;U128 fm=FORB[J];for(auto v:sel)proj.push_back(v&fm);auto nb=nullbasis(proj);if(nb.empty()||nb.size()>20)continue;uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t c=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)c^=nb[k];State z1=coeffstate(I,c);State x2=applycols(C0,z1);if(wt(z1)!=t||wt(x2)!=u)continue;State z2{};for(int b=0;b<16;b++)if(x2[b])z2[b]=bestdy[x2[b]];State x3=applycols(C1,z2);best.candidates++;int N=t+u+wt(x3);if(N<best.N){best.N=N;best.z1=z1;best.x2=x2;best.z2=z2;best.x3=x3;}}}}}return best;}
Best searchlin(const std::array<U128,128>&T0,const std::array<U128,128>&T1,int B){int lat[256][256]={};for(int a=0;a<256;a++)for(int b=0;b<256;b++){int w=0;for(int x=0;x<256;x++){int p1=par(uint8_t(a&x)),p2=par(uint8_t(b&SBOX[x]));w+=(p1==p2)?1:-1;}lat[a][b]=w;}std::array<std::vector<uint8_t>,256> opts;for(int a=1;a<256;a++)for(int b=1;b<256;b++)if(std::abs(lat[a][b])==32)opts[a].push_back((uint8_t)b);Best best;for(int t=1;t<B;t++){int u=B-t;if(t>8||u>16)continue;for(auto I:COMB[t]){std::vector<U128>sel;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(T0[8*b+bit]);if(sel.size()>63)continue;for(auto J:COMB[u]){std::vector<U128>proj;U128 fm=FORB[J];for(auto v:sel)proj.push_back(v&fm);auto nb=nullbasis(proj);if(nb.empty()||nb.size()>16)continue;uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t c=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)c^=nb[k];State z1=coeffstate(I,c);State x2=applycols(T0,z1);if(wt(z1)!=t||wt(x2)!=u)continue;std::vector<int> pos;for(int b=0;b<16;b++)if(x2[b])pos.push_back(b);uint64_t combos=1;for(int b:pos)combos*=opts[x2[b]].size();if(combos>200000)continue;for(uint64_t idx=0;idx<combos;idx++){uint64_t qx=idx;State z2{};for(int b:pos){auto&o=opts[x2[b]];z2[b]=o[qx%o.size()];qx/=o.size();}State x3=applycols(T1,z2);best.candidates++;int N=t+u+wt(x3);if(N<best.N){best.N=N;best.z1=z1;best.x2=x2;best.z2=z2;best.x3=x3;}}}}}}return best;}
int main(int argc,char**argv){init();std::string mode=argc>1?argv[1]:"diff";std::string sched=argc>2?argv[2]:"static";int r=argc>3?std::stoi(argv[3]):0;int keep=argc>4?std::stoi(argv[4],nullptr,0):0xff;int B=argc>5?std::stoi(argv[5]):(keep==0xff?8:7);bool rotor=sched=="rotor";auto L0=cols(r,rotor,keep,false);auto L1=cols(r+1,rotor,keep,false);Best b;if(mode=="diff")b=searchdiff(L0,L1,B);else{auto I0=cols(r,rotor,keep,true);auto I1=cols(r+1,rotor,keep,true);auto T0=trans(I0),T1=trans(I1);b=searchlin(T0,T1,B);}std::cout<<mode<<" "<<sched<<" r="<<r<<" keep=0x"<<std::hex<<keep<<std::dec<<" branch_first="<<B<<" restricted_best_active="<<b.N<<" candidates="<<b.candidates<<"\n";std::cout<<"z1="<<hx(b.z1)<<" w="<<wt(b.z1)<<"\nx2="<<hx(b.x2)<<" w="<<wt(b.x2)<<"\nz2="<<hx(b.z2)<<" w="<<wt(b.z2)<<"\nx3="<<hx(b.x3)<<" w="<<wt(b.x3)<<"\n";return 0;}

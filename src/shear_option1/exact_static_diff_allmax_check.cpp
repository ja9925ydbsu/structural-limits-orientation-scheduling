// Exact all-DDT=4 feasibility checker for the full static Option 1 layer.
//
// For a requested three-round activity split a->b->c, this enumerates every
// exact first linear relation z1 -> x2 and applies the unique AES DDT-entry-4
// output difference to each active byte of x2. It then tests whether the
// second static shear layer yields exactly c active bytes. Running this over
// all activity-12 splits, together with the exact one-active inverse check,
// excludes a 2^-72 three-round characteristic.
// Option 1 branch only; not part of the structural-limits paper.

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82}; static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256>SBOX, BESTDY;
struct Step{int t,s,k;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;} uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++)y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c);return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;} void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<8;pi++)cell(v,ps[pi].first,ps[pi].second,0);}return v;}
State layer(State x){auto v=steps();for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);return x;} U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;} State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=uint8_t(v&255);v>>=8;}return x;}
std::array<U128,128> cols(){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x));}return C;} State applycols(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);} U128 bmask(int b){return (U128)255<<(8*(15-b));} int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 127-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;}
int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;} uint16_t supp(const State&x){uint16_t s=0;for(int b=0;b<16;b++)if(x[b])s|=uint16_t(1u<<b);return s;}
std::vector<uint32_t> nullbasis(const std::vector<U128>&vs){U128 bas[128]={};uint32_t cf[128]={};std::vector<uint32_t>N;for(int j=0;j<(int)vs.size();j++){U128 x=vs[j];uint32_t c=1u<<j;while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}if(!x)N.push_back(c);}return N;}
State coeffstate(uint16_t I,uint32_t c){State x{};int p=0;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=uint8_t(1u<<(7-bit));p++;}return x;}
uint8_t gm(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb(x);int ddt[256][256]={};for(int dx=0;dx<256;dx++)for(int x=0;x<256;x++)ddt[dx][SBOX[x]^SBOX[x^dx]]++;BESTDY[0]=0;for(int dx=1;dx<256;dx++){int n=0;for(int dy=1;dy<256;dy++)if(ddt[dx][dy]==4){BESTDY[dx]=dy;n++;}if(n!=1){std::cerr<<"DDT4 count dx="<<dx<<" n="<<n<<"\n";std::exit(2);}}for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}
int main(int argc,char**argv){init();int a=argc>1?std::stoi(argv[1]):4,b=argc>2?std::stoi(argv[2]):4,c=argc>3?std::stoi(argv[3]):4;auto C=cols();uint64_t pairs_checked=0, rels=0;for(auto I:COMB[a]){std::vector<U128>sel;sel.reserve(8*a);for(int by=0;by<16;by++)if((I>>by)&1)for(int bit=0;bit<8;bit++)sel.push_back(C[8*by+bit]);for(auto K:COMB[b]){pairs_checked++;std::vector<U128>p;p.reserve(sel.size());U128 fm=FORB[K];for(auto v:sel)p.push_back(v&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>24){std::cerr<<"nullity too high "<<nb.size()<<" I="<<I<<" K="<<K<<"\n";return 3;}uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint32_t co=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)co^=nb[k];State z1=coeffstate(I,co);if(wt(z1)!=a)continue;State x2=applycols(C,z1);if(wt(x2)!=b||supp(x2)!=K)continue;rels++;State z2{};for(int by=0;by<16;by++)if(x2[by])z2[by]=BESTDY[x2[by]];State x3=applycols(C,z2);if(wt(x3)==c){std::cout<<"FOUND all-DDT4 split="<<a<<"->"<<b<<"->"<<c<<" pairs="<<pairs_checked<<" rels="<<rels<<"\n";return 1;}}}}std::cout<<"EXCLUDED all-DDT4 split="<<a<<"->"<<b<<"->"<<c<<" pairs="<<pairs_checked<<" rels="<<rels<<"\n";return 0;}

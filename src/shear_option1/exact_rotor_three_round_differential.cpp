// Exact three-round differential characteristic optimizer for the full Option 1 rotor layer.
//
// For a requested rotor phase, total active-S-box count, and either one split
// a->b->c or all branch-admissible splits, this checker enumerates exact GF(2)
// relations on both sides of the common middle-byte support and evaluates the
// actual AES DDT values. For AES, every nonzero DDT entry is 2 or 4, so an
// N-active characteristic has exact cost 6*N plus one bit for each active
// middle transition with DDT entry 2.
//
// The program distinguishes:
//   NO_EXACT_SUPPORT          no exact support path exists;
//   SUPPORT_BUT_NO_DDT       exact support paths exist but none are DDT-compatible;
//   DDT_COMPAT               compatible paths exist, with exact minimum cost.
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
#include <algorithm>
#ifdef _OPENMP
#include <omp.h>
#endif
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82};
static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256>SBOX; static uint8_t DDT[256][256];
struct Step{int t,s,k;}; struct Rel{State a,b;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;}
uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++)y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=uint8_t(b[r][c]<<(7-c));return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;}
void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(int round){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<8;pi++){int k=(round+st+pi)&3;cell(v,ps[pi].first,ps[pi].second,k);}}return v;}
State layer(State x,int round,bool inv=false){auto v=steps(round);if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}else{for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);}return x;}
U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;} State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=uint8_t(v&255);v>>=8;}return x;}
std::array<U128,128> cols(int round,bool inv=false){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x,round,inv));}return C;}
State applyC(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);}
U128 bmask(int b){return(U128)255<<(8*(15-b));} int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 64+63-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;} int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;} uint16_t supp(const State&x){uint16_t s=0;for(int b=0;b<16;b++)if(x[b])s|=uint16_t(1u<<b);return s;}
std::string hx(const State&x){static const char*d="0123456789abcdef";std::string s;for(auto b:x){s+=d[b>>4];s+=d[b&15];}return s;}
std::vector<uint64_t> nullbasis(const std::vector<U128>&vs){U128 bas[128]={};uint64_t cf[128]={};std::vector<uint64_t>N;if(vs.size()>63)throw std::runtime_error("coeff>63");for(int j=0;j<(int)vs.size();j++){U128 x=vs[j];uint64_t c=1ULL<<j;while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}if(!x)N.push_back(c);}return N;}
State coeffstate(uint16_t I,uint64_t c){State x{};int p=0;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=uint8_t(1u<<(7-bit));p++;}return x;}
bool independent(const std::vector<U128>&vs){U128 bas[128]={};for(auto x:vs){while(x){int b=top(x);if(bas[b])x^=bas[b];else{bas[b]=x;break;}}if(!x)return false;}return true;}
bool envelope_possible(const std::array<U128,128>&C,int iw,uint16_t J){U128 fm=FORB[J];for(auto I:COMB[iw]){std::vector<U128>p;p.reserve(8*iw);for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);if(!independent(p))return true;}return false;}
std::vector<Rel> to_output(const std::array<U128,128>&C,int iw,uint16_t J){int ow=__builtin_popcount((unsigned)J);std::vector<Rel>R;U128 fm=FORB[J];for(auto I:COMB[iw]){std::vector<U128>p;p.reserve(8*iw);for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>24)throw std::runtime_error("nullity>24");uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t cc=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)cc^=nb[k];State a=coeffstate(I,cc);if(wt(a)!=iw)continue;State b=applyC(C,a);if(wt(b)==ow&&supp(b)==J)R.push_back({a,b});}}return R;}
uint8_t gm(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb((uint8_t)x);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}int d[256][256]={};for(int dx=0;dx<256;dx++)for(int x=0;x<256;x++)d[dx][SBOX[x]^SBOX[x^dx]]++;for(int a=0;a<256;a++)for(int b=0;b<256;b++)DDT[a][b]=uint8_t(d[a][b]);}
int main(int argc,char**argv){if(argc<4){std::cerr<<"usage: phase total all | phase total a b c\n";return 2;}init();int phase=std::stoi(argv[1]),total=std::stoi(argv[2]);std::vector<std::array<int,3>> cand;if(std::string(argv[3])=="all"){for(int a=1;a<total;a++)for(int b=1;b<total;b++){int c=total-a-b;if(c<1)continue;if(a>7||c>7)continue;if(a+b<8||b+c<8)continue;cand.push_back({a,b,c});}}else{if(argc<6)return 2;cand.push_back({std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5])});}
auto C0=cols(phase,false), C1inv=cols(phase+1,true);int global=999;std::array<int,3>gsp{};State gz1{},gx2{},gz2{},gx3{};long long gpairs=0,gcompat=0;int support_splits=0,compat_splits=0;
for(auto sp:cand){int a=sp[0],b=sp[1],c=sp[2];auto Ks=COMB[b];long long splitpairs=0,splitcompat=0;int splitbest=999;State bz1{},bx2{},bz2{},bx3{};bool support=false;
#pragma omp parallel for schedule(dynamic,1)
for(long long ki=0;ki<(long long)Ks.size();ki++){uint16_t K=Ks[ki];if(!envelope_possible(C0,a,K)||!envelope_possible(C1inv,c,K))continue;auto L=to_output(C0,a,K);if(L.empty())continue;auto R=to_output(C1inv,c,K);if(R.empty())continue;long long lp=0,lc=0;int lb=999;State lz1{},lx2{},lz2{},lx3{};for(const auto&l:L)for(const auto&r:R){lp++;int pen=0;bool ok=true;for(int by=0;by<16;by++)if(l.b[by]){int v=DDT[l.b[by]][r.b[by]];if(v==0){ok=false;break;}if(v==2)pen++;else if(v!=4){std::cerr<<"unexpected DDT="<<v<<"\n";std::abort();}}if(!ok)continue;lc++;int cost=6*total+pen;if(cost<lb){lb=cost;lz1=l.a;lx2=l.b;lz2=r.b;lx3=r.a;}}
#pragma omp critical
{splitpairs+=lp;splitcompat+=lc;if(lp) support=true;if(lb<splitbest){splitbest=lb;bz1=lz1;bx2=lx2;bz2=lz2;bx3=lx3;}}}
if(support)support_splits++;if(splitcompat)compat_splits++;gpairs+=splitpairs;gcompat+=splitcompat;std::cout<<"split="<<a<<"->"<<b<<"->"<<c<<" ";if(!support)std::cout<<"NO_EXACT_SUPPORT";else if(!splitcompat)std::cout<<"SUPPORT_BUT_NO_DDT";else std::cout<<"DDT_COMPAT cost_bits="<<splitbest<<" penalty2="<<(splitbest-6*total);std::cout<<" pairs="<<splitpairs<<" compatible="<<splitcompat<<"\n";if(splitbest<global){global=splitbest;gsp=sp;gz1=bz1;gx2=bx2;gz2=bz2;gx3=bx3;}}
std::cout<<"SUMMARY phase="<<phase<<" total="<<total<<" splits="<<cand.size()<<" support_splits="<<support_splits<<" compatible_splits="<<compat_splits<<" pairs="<<gpairs<<" compatible="<<gcompat; if(global<999)std::cout<<" best_cost="<<global<<" split="<<gsp[0]<<"->"<<gsp[1]<<"->"<<gsp[2]<<"\nz1="<<hx(gz1)<<"\nx2="<<hx(gx2)<<"\nz2="<<hx(gz2)<<"\nx3="<<hx(gx3); else std::cout<<" NO_DDT_COMPATIBLE_TRAIL"; std::cout<<"\n";return 0;}

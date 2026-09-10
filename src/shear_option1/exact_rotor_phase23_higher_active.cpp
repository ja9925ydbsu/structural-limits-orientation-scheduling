// Exact higher-activity linear-trail competitor checker for full Option 1 rotor phases.
//
// This parameterized checker is used after the exact 13-active optimization.
// It exhausts exact GF(2) relations for a requested total activity and tests
// every compatible AES LAT transition whose cost can beat a supplied target.
// Branch-number and one-active endpoint constraints are applied before value
// enumeration. OpenMP only parallelizes independent middle-support checks.
//
// Usage:
//   ./exact_rotor_phase23_higher_active phase total target_bits all
//   ./exact_rotor_phase23_higher_active phase total target_bits a b c
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
static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256>SBOX; static double LATC[256][256];
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
std::array<U128,128> trans(const std::array<U128,128>&C){std::array<U128,128>T{};for(int j=0;j<128;j++){U128 v=0;for(int r=0;r<128;r++)if((C[r]>>(127-j))&1)v|=(U128)1<<(127-r);T[j]=v;}return T;}
State applyC(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);}
U128 bmask(int b){return(U128)255<<(8*(15-b));} int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 64+63-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;} int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;} uint16_t supp(const State&x){uint16_t s=0;for(int b=0;b<16;b++)if(x[b])s|=uint16_t(1u<<b);return s;}
std::vector<uint64_t> nullbasis(const std::vector<U128>&vs){U128 bas[128]={};uint64_t cf[128]={};std::vector<uint64_t>N;if(vs.size()>63)throw std::runtime_error("coeff>63");for(int j=0;j<(int)vs.size();j++){U128 x=vs[j];uint64_t c=1ULL<<j;while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}if(!x)N.push_back(c);}return N;}
State coeffstate(uint16_t I,uint64_t c){State x{};int p=0;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=uint8_t(1u<<(7-bit));p++;}return x;}
bool independent(const std::vector<U128>&vs){U128 bas[128]={};for(auto x:vs){while(x){int b=top(x);if(bas[b])x^=bas[b];else{bas[b]=x;break;}}if(!x)return false;}return true;}
bool envelope_possible(const std::array<U128,128>&C,int iw,uint16_t J){U128 fm=FORB[J];for(auto I:COMB[iw]){std::vector<U128>p;p.reserve(8*iw);for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);if(!independent(p))return true;}return false;}
std::vector<Rel> to_output(const std::array<U128,128>&C,int iw,uint16_t J){int ow=__builtin_popcount((unsigned)J);std::vector<Rel>R;U128 fm=FORB[J];for(auto I:COMB[iw]){std::vector<U128>p;p.reserve(8*iw);for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*b+bit]&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>24)throw std::runtime_error("nullity>24");uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t cc=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)cc^=nb[k];State a=coeffstate(I,cc);if(wt(a)!=iw)continue;State b=applyC(C,a);if(wt(b)==ow&&supp(b)==J)R.push_back({a,b});}}return R;}
uint8_t gm(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb((uint8_t)x);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}for(int a=0;a<256;a++)for(int b=0;b<256;b++)LATC[a][b]=std::numeric_limits<double>::infinity();for(int a=1;a<256;a++)for(int b=1;b<256;b++){int w=0;for(int x=0;x<256;x++)w+=(par(uint8_t(a&x))==par(uint8_t(b&SBOX[x])))?1:-1;if(w)LATC[a][b]=8.0-std::log2((double)std::abs(w));}}
int main(int argc,char**argv){if(argc<5){std::cerr<<"usage: phase total target_bits a b c OR phase total target_bits all\n";return 2;}init();int phase=std::stoi(argv[1]);int total=std::stoi(argv[2]);double target=std::stod(argv[3]);std::vector<std::array<int,3>> cand;if(std::string(argv[4])=="all"){for(int a=2;a<=7;a++)for(int b=1;b<=12;b++){int c=total-a-b;if(c<2||c>7)continue;if(a+b<8||b+c<8)continue;if(a+b==8&&!(a==4&&b==4))continue;if(b+c==8&&!(b==4&&c==4))continue;cand.push_back({a,b,c});}}else{if(argc<7)return 2;cand.push_back({std::stoi(argv[4]),std::stoi(argv[5]),std::stoi(argv[6])});}
auto T0=trans(cols(phase,true)), T1inv=trans(cols(phase+1,false));bool found=false;double best=target;long long allpairs=0;std::array<int,3>bsp{};for(auto sp:cand){int a=sp[0],b=sp[1],c=sp[2];double middle_limit=target-3.0*(a+c);long long splitpairs=0;double splitbest=1e99;bool splitfound=false;auto Ks=COMB[b];
#pragma omp parallel for schedule(dynamic,1)
for(long long ki=0;ki<(long long)Ks.size();ki++){uint16_t K=Ks[ki];if(!envelope_possible(T0,a,K)||!envelope_possible(T1inv,c,K))continue;auto L=to_output(T0,a,K);if(L.empty())continue;auto R=to_output(T1inv,c,K);if(R.empty())continue;long long lp=0;double lb=1e99;bool lf=false;for(const auto&l:L)for(const auto&r:R){lp++;double mc=0;bool ok=true;int seen=0;for(int by=0;by<16;by++)if(l.b[by]){double q=LATC[l.b[by]][r.b[by]];if(!std::isfinite(q)){ok=false;break;}mc+=q;++seen;if(mc+3.0*(b-seen)>=middle_limit-1e-12){ok=false;break;}}if(!ok)continue;double tc=3.0*(a+c)+mc;if(tc<lb){lb=tc;lf=true;}}
#pragma omp critical
{splitpairs+=lp;if(lf&&lb<splitbest){splitbest=lb;splitfound=true;}if(lf&&lb<best){best=lb;found=true;bsp=sp;}}}
allpairs+=splitpairs;std::cout<<"split="<<a<<"->"<<b<<"->"<<c<<" ";if(splitfound)std::cout<<"FOUND_BELOW target best="<<std::setprecision(15)<<splitbest;else std::cout<<"excluded_below_target";std::cout<<" pairs="<<splitpairs<<"\n";}
if(found){std::cout<<"FOUND phase="<<phase<<" total="<<total<<" best="<<std::setprecision(15)<<best<<" split="<<bsp[0]<<"->"<<bsp[1]<<"->"<<bsp[2]<<"\n";return 1;}std::cout<<"EXCLUDED phase="<<phase<<" total="<<total<<" below="<<std::setprecision(15)<<target<<" splits="<<cand.size()<<" pairs="<<allpairs<<"\n";return 0;}

// Broadened three-round value-sensitive trail search for Option 1.
//
// This checker exhaustively enumerates all first inter-round states that attain
// the exact byte branch number, then allows every nonzero AES DDT/LAT
// transition at the middle S-box layer. It minimizes the actual differential
// -log2 probability or linear -log2 correlation magnitude, rather than only
// counting active S-boxes.
//
// IMPORTANT: the result is exact only inside the stated class: the first
// linear transition must attain the byte branch number. A separate probe is
// used for non-branch-minimal first transitions.

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <iomanip>
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82}; static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256>SBOX;
struct Step{int t,s,k;};
int par(uint8_t x){return __builtin_popcount((unsigned)x)&1;} uint8_t am(const M8&m,uint8_t x){uint8_t y=0;for(int i=0;i<8;i++)y|=uint8_t(par(m[i]&x)<<(7-i));return y;}
M8 rot(const M8&m){int a[8][8]={},b[8][8]={};for(int r=0;r<8;r++)for(int c=0;c<8;c++)a[r][c]=(m[r]>>(7-c))&1;for(int r=0;r<8;r++)for(int c=0;c<8;c++)b[r][c]=a[7-c][r];M8 o{};for(int r=0;r<8;r++)for(int c=0;c<8;c++)o[r]|=b[r][c]<<(7-c);return o;}
std::vector<std::pair<int,int>> pairs(int mask){std::vector<std::pair<int,int>>v;for(int i=0;i<16;i++)if(i<(i^mask))v.push_back({i,i^mask});return v;} void cell(std::vector<Step>&v,int l,int r,int k){v.push_back({l,r,k});v.push_back({r,l,k^1});v.push_back({l,r,k});}
std::vector<Step> steps(int round,bool rotor,int keep){std::vector<Step>v;int masks[4]={1,2,4,8};for(int st=0;st<4;st++){auto ps=pairs(masks[st]);for(int pi=0;pi<8;pi++){if(st==3&&!((keep>>pi)&1))continue;int k=rotor?((round+st+pi)&3):0;cell(v,ps[pi].first,ps[pi].second,k);}}return v;}
State layer(State x,int round,bool rotor,int keep,bool inv=false){auto v=steps(round,rotor,keep);if(!inv){for(auto&s:v)x[s.t]^=am(MF[s.k],x[s.s]);}else for(auto it=v.rbegin();it!=v.rend();++it)x[it->t]^=am(MF[it->k],x[it->s]);return x;}
U128 pack(const State&x){U128 v=0;for(auto b:x)v=(v<<8)|b;return v;} State unpack(U128 v){State x{};for(int i=15;i>=0;i--){x[i]=uint8_t(v&255);v>>=8;}return x;}
std::array<U128,128> cols(int round,bool rotor,int keep,bool inv=false){std::array<U128,128>C{};for(int j=0;j<128;j++){State x{};x[j/8]=uint8_t(1u<<(7-(j%8)));C[j]=pack(layer(x,round,rotor,keep,inv));}return C;}
std::array<U128,128> trans(const std::array<U128,128>&C){std::array<U128,128>T{};for(int j=0;j<128;j++){U128 v=0;for(int r=0;r<128;r++)if((C[r]>>(127-j))&1)v|=(U128)1<<(127-r);T[j]=v;}return T;}
State applycols(const std::array<U128,128>&C,const State&x){U128 y=0;for(int b=0;b<16;b++)for(int bit=0;bit<8;bit++)if((x[b]>>(7-bit))&1)y^=C[8*b+bit];return unpack(y);} U128 bmask(int b){return (U128)255<<(8*(15-b));} int top(U128 x){uint64_t h=(uint64_t)(x>>64);if(h)return 127-__builtin_clzll(h);uint64_t l=(uint64_t)x;return l?63-__builtin_clzll(l):-1;}
int wt(const State&x){int n=0;for(auto b:x)n+=b!=0;return n;} uint16_t supp(const State&x){uint16_t s=0;for(int b=0;b<16;b++)if(x[b])s|=uint16_t(1u<<b);return s;} std::string hx(const State&x){static const char*d="0123456789abcdef";std::string s;for(auto b:x){s+=d[b>>4];s+=d[b&15];}return s;}
std::vector<uint64_t> nullbasis(const std::vector<U128>&vs){U128 bas[128]={};uint64_t cf[128]={};std::vector<uint64_t>N;for(int j=0;j<(int)vs.size();j++){U128 x=vs[j];uint64_t c=1ULL<<j;while(x){int b=top(x);if(bas[b]){x^=bas[b];c^=cf[b];}else{bas[b]=x;cf[b]=c;break;}}if(!x)N.push_back(c);}return N;}
State coeffstate(uint16_t I,uint64_t c){State x{};int p=0;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++){if((c>>p)&1)x[b]|=uint8_t(1u<<(7-bit));p++;}return x;}
uint8_t gm(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb(x);for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}
struct Rel{State a,b;};
std::vector<Rel> branchrels(const std::array<U128,128>&C,int B){std::vector<Rel> out;for(int t=1;t<B;t++){int u=B-t;if(B==8&&t!=4)continue;if(B==7&&t!=3&&t!=4)continue;if(t>8)continue;for(auto I:COMB[t]){std::vector<U128>sel;for(int b=0;b<16;b++)if((I>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(C[8*b+bit]);for(auto J:COMB[u]){std::vector<U128>p;U128 fm=FORB[J];for(auto v:sel)p.push_back(v&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>20)throw std::runtime_error("nullspace dimension exceeds exhaustive enumeration limit");uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t c=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)c^=nb[k];State a=coeffstate(I,c);if(wt(a)!=t)continue;State b=applycols(C,a);if(wt(b)==u)out.push_back({a,b});}}}}return out;}
struct Best{double score=1e99,mid=0;State z1{},x2{},z2{},x3{};long long tested=0;bool found=false;};
int main(int argc,char**argv){init();std::string mode=argc>1?argv[1]:"linear";std::string sched=argc>2?argv[2]:"rotor";int r=argc>3?std::stoi(argv[3]):0;int keep=argc>4?std::stoi(argv[4],nullptr,0):0xff;int B=argc>5?std::stoi(argv[5]):(keep==0xff?8:7);int kmax=argc>6?std::stoi(argv[6]):12;bool rotor=sched=="rotor",linear=mode=="linear";int ddt[256][256]={},lat[256][256]={};for(int dx=0;dx<256;dx++)for(int x=0;x<256;x++)ddt[dx][SBOX[x]^SBOX[x^dx]]++;for(int a=0;a<256;a++)for(int b=0;b<256;b++){int w=0;for(int x=0;x<256;x++)w+=(par(uint8_t(a&x))==par(uint8_t(b&SBOX[x])))?1:-1;lat[a][b]=w;}
auto L0=cols(r,rotor,keep,false),L1=cols(r+1,rotor,keep,false);std::array<U128,128>C0=linear?trans(cols(r,rotor,keep,true)):L0,C1=linear?trans(cols(r+1,rotor,keep,true)):L1;auto R=branchrels(C0,B);std::unordered_map<uint16_t,std::vector<Rel>> groups;for(auto&z:R)groups[supp(z.b)].push_back(z);Best global;double edge=linear?3.0:6.0;
for(int kout=1;kout<=kmax;kout++){double floor=edge*(B+kout);if(global.found&&floor>=global.score-1e-12)break;Best bestk;for(auto&gg:groups){uint16_t K=gg.first;int m=__builtin_popcount((unsigned)K);if(m+kout<B)continue;std::vector<U128>sel;for(int b=0;b<16;b++)if((K>>b)&1)for(int bit=0;bit<8;bit++)sel.push_back(C1[8*b+bit]);for(auto J:COMB[kout]){std::vector<U128>p;U128 fm=FORB[J];for(auto v:sel)p.push_back(v&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>20)throw std::runtime_error("nullspace dimension exceeds exhaustive enumeration limit");uint64_t total=1ULL<<nb.size();for(uint64_t q=1;q<total;q++){uint64_t c=0;for(int kk=0;kk<(int)nb.size();kk++)if((q>>kk)&1)c^=nb[kk];State z2=coeffstate(K,c);if(wt(z2)!=m)continue;State x3=applycols(C1,z2);if(wt(x3)!=kout)continue;for(auto&r0:gg.second){bestk.tested++;double mc=0;bool ok=true;for(int b=0;b<16;b++)if(r0.b[b]){if(!linear){int v=ddt[r0.b[b]][z2[b]];if(!v){ok=false;break;}mc+=8-std::log2((double)v);}else{int v=std::abs(lat[r0.b[b]][z2[b]]);if(!v){ok=false;break;}mc+=8-std::log2((double)v);}}if(!ok)continue;double sc=edge*wt(r0.a)+mc+edge*kout;if(sc<bestk.score)bestk={sc,mc,r0.a,r0.b,z2,x3,bestk.tested,true};}}}}if(bestk.found&&bestk.score<global.score)global=bestk;}
std::cout<<"RESULT mode="<<mode<<" schedule="<<sched<<" round="<<r<<" keep=0x"<<std::hex<<keep<<std::dec<<" B="<<B;if(global.found){std::cout<<" cost_bits="<<std::fixed<<std::setprecision(6)<<global.score<<" active="<<wt(global.z1)+wt(global.x2)+wt(global.x3)<<" split="<<wt(global.z1)<<"->"<<wt(global.x2)<<"->"<<wt(global.x3)<<" middle_cost="<<global.mid<<"\n"<<"z1="<<hx(global.z1)<<"\nx2="<<hx(global.x2)<<"\nz2="<<hx(global.z2)<<"\nx3="<<hx(global.x3)<<"\n";}else std::cout<<" no_trail\n";}

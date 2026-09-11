// Exact all-DDT=4 feasibility checker for full Option 1 rotor phases.
//
// AES has exactly one DDT-entry-4 output for each nonzero input difference and
// exactly one inverse choice for each nonzero output difference. Therefore a
// candidate characteristic at the local probability floor is deterministic
// once one side of the linear relation is fixed. This checker chooses the
// smaller endpoint direction, enumerates exact GF(2) relations, applies that
// unique DDT=4 map (or its inverse), and tests the remaining linear layer.
//
// Usage: ./exact_rotor_differential_allmax phase total a b c
// Option 1 branch only; not part of the structural-limits paper under review.

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif
using U128=__uint128_t; using M8=std::array<uint8_t,8>; using State=std::array<uint8_t,16>;
static const M8 BASE={0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82};
static std::array<M8,4> MF; static std::array<std::vector<uint16_t>,17> COMB; static std::array<U128,1u<<16> FORB; static std::array<uint8_t,256>SBOX; static int DDT[256][256];
struct Step{int t,s,k;};
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
uint8_t gm(uint8_t a,uint8_t b){uint8_t o=0;for(int i=0;i<8;i++){if(b&1)o^=a;bool c=a&128;a<<=1;if(c)a^=0x1b;b>>=1;}return o;} uint8_t gp(uint8_t a,int e){uint8_t o=1;while(e){if(e&1)o=gm(o,a);a=gm(a,a);e>>=1;}return o;} uint8_t rol(uint8_t x,int n){return uint8_t((x<<n)|(x>>(8-n)));} uint8_t sb(uint8_t x){uint8_t v=x?gp(x,254):0;return v^rol(v,1)^rol(v,2)^rol(v,3)^rol(v,4)^0x63;}
void init(){MF[0]=BASE;for(int k=1;k<4;k++)MF[k]=rot(MF[k-1]);for(int x=0;x<256;x++)SBOX[x]=sb((uint8_t)x);for(int dx=0;dx<256;dx++)for(int x=0;x<256;x++)DDT[dx][SBOX[x]^SBOX[x^dx]]++;for(uint32_t m=0;m<(1u<<16);m++)COMB[__builtin_popcount(m)].push_back((uint16_t)m);U128 all=~(U128)0;for(uint32_t J=0;J<(1u<<16);J++){U128 f=all;for(int b=0;b<16;b++)if((J>>b)&1)f^=bmask(b);FORB[J]=f;}}
static uint8_t F4[256],R4[256];
void init4(){for(int dx=1;dx<256;dx++){int n=0,dy0=0;for(int dy=1;dy<256;dy++)if(DDT[dx][dy]==4){n++;dy0=dy;}if(n!=1){std::cerr<<"row4 n="<<n<<" dx="<<dx<<"\n";std::exit(3);}F4[dx]=dy0;}for(int dy=1;dy<256;dy++){int n=0,dx0=0;for(int dx=1;dx<256;dx++)if(DDT[dx][dy]==4){n++;dx0=dx;}if(n!=1){std::cerr<<"col4 n="<<n<<" dy="<<dy<<"\n";std::exit(4);}R4[dy]=dx0;}}
int main(int argc,char**argv){if(argc<6){std::cerr<<"usage: phase total a b c\n";return 2;}init();init4();int phase=std::stoi(argv[1]),total=std::stoi(argv[2]),a=std::stoi(argv[3]),b=std::stoi(argv[4]),c=std::stoi(argv[5]);if(a+b+c!=total)return 2;auto L0=cols(phase,false),L0i=cols(phase,true),L1=cols(phase+1,false),L1i=cols(phase+1,true);bool backward=c<a;auto C=backward?L1i:L0;int iw=backward?c:a;auto Ks=COMB[b];int found=0;long long rels=0;State w0{},w1{},w2{},w3{};uint16_t wK=0;
#pragma omp parallel for schedule(dynamic,1)
for(long long ki=0;ki<(long long)Ks.size();ki++){if(found)continue;uint16_t K=Ks[ki];U128 fm=FORB[K];for(auto I:COMB[iw]){if(found)break;std::vector<U128>p;p.reserve(8*iw);for(int by=0;by<16;by++)if((I>>by)&1)for(int bit=0;bit<8;bit++)p.push_back(C[8*by+bit]&fm);auto nb=nullbasis(p);if(nb.empty())continue;if(nb.size()>24)throw std::runtime_error("nullity>24");uint64_t lim=1ULL<<nb.size();for(uint64_t q=1;q<lim;q++){if(found)break;uint64_t cc=0;for(int k=0;k<(int)nb.size();k++)if((q>>k)&1)cc^=nb[k];State s=coeffstate(I,cc);if(wt(s)!=iw)continue;State mid=applyC(C,s);if(wt(mid)!=b||supp(mid)!=K)continue;
#pragma omp atomic
rels++;
if(!backward){State z2{};for(int by=0;by<16;by++)if(mid[by])z2[by]=F4[mid[by]];State x3=applyC(L1,z2);if(wt(x3)==c){
#pragma omp critical
{if(!found){found=1;w0=s;w1=mid;w2=z2;w3=x3;wK=K;}}}}
else{State x2{};for(int by=0;by<16;by++)if(mid[by])x2[by]=R4[mid[by]];State z1=applyC(L0i,x2);if(wt(z1)==a){
#pragma omp critical
{if(!found){found=1;w0=z1;w1=x2;w2=mid;w3=s;wK=K;}}}}
}}}
std::cout<<"phase="<<phase<<" total="<<total<<" split="<<a<<"->"<<b<<"->"<<c<<" direction="<<(backward?"backward":"forward")<<" exact_relations_tested="<<rels<<" all_DDT4="<<(found?"FOUND":"EXCLUDED");if(found)std::cout<<" K=0x"<<std::hex<<wK<<std::dec<<"\nz1="<<hx(w0)<<"\nx2="<<hx(w1)<<"\nz2="<<hx(w2)<<"\nx3="<<hx(w3);std::cout<<"\n";return found?1:0;}

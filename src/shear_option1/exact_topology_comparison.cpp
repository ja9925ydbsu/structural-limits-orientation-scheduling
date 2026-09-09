// Exact topology comparison for the Option 1 cross-byte shear redesign.
//
// Topologies:
//   chain       : (0,1),(1,2),...,(14,15)                         15 cells
//   ring        : chain plus (15,0)                              16 cells
//   butterfly3  : masks 1,2,4                                   24 cells
//   butterfly4  : masks 1,2,4,8                                 32 cells
//
// Every lifting cell contains three byte shears.  The static schedule uses
// k=0 for each cell.  The rotor schedule uses k=(round+stage+pair_index) mod 4;
// for chain/ring, each sequential edge is treated as a one-pair stage.
//
// Branch numbers are obtained by exhaustive support-set GF(2) rank tests,
// using the same exact criterion as exact_branch_number.cpp.

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using U128 = __uint128_t;
using Matrix8 = std::array<std::uint8_t,8>;
using State = std::array<std::uint8_t,16>;

static const Matrix8 BASE_MATRIX = {
    0xC8,0x73,0xF5,0x14,0x01,0xC0,0xDE,0x82
};
static std::array<Matrix8,4> MATRIX_FAMILY;
static std::array<std::vector<std::uint16_t>,17> COMBINATIONS;
static std::array<U128,1u<<16> FORBIDDEN_MASKS;

struct Step { int target, source, orientation; };
struct SearchResult {
    bool found=false, inverse=false;
    int input_bytes=0, output_bytes=0;
    std::uint64_t checks=0;
};

static int parity8(std::uint8_t x) {
    return __builtin_popcount(static_cast<unsigned>(x)) & 1;
}
static std::uint8_t apply_matrix8(const Matrix8& m, std::uint8_t x) {
    std::uint8_t y=0;
    for (int i=0;i<8;++i) y|=static_cast<std::uint8_t>(parity8(m[i]&x)<<(7-i));
    return y;
}
static Matrix8 rotate_clockwise(const Matrix8& m) {
    int a[8][8]={}, b[8][8]={};
    for (int r=0;r<8;++r) for (int c=0;c<8;++c) a[r][c]=(m[r]>>(7-c))&1;
    for (int r=0;r<8;++r) for (int c=0;c<8;++c) b[r][c]=a[7-c][r];
    Matrix8 out{};
    for (int r=0;r<8;++r) for (int c=0;c<8;++c) out[r]|=b[r][c]<<(7-c);
    return out;
}
static std::vector<std::pair<int,int>> butterfly_pairs(int mask) {
    std::vector<std::pair<int,int>> out;
    for (int i=0;i<16;++i) if (i<(i^mask)) out.push_back({i,i^mask});
    return out;
}
static void add_cell(std::vector<Step>& out, int left, int right, int k) {
    out.push_back({left,right,k});
    out.push_back({right,left,k^1});
    out.push_back({left,right,k});
}
static std::vector<Step> layer_steps(
    const std::string& topology, int round, bool rotor) {
    std::vector<Step> out;
    if (topology=="butterfly3" || topology=="butterfly4") {
        const int masks[4]={1,2,4,8};
        const int stages=topology=="butterfly4" ? 4 : 3;
        for (int stage=0;stage<stages;++stage) {
            const auto pairs=butterfly_pairs(masks[stage]);
            for (int pair_index=0;pair_index<static_cast<int>(pairs.size());++pair_index) {
                const auto [left,right]=pairs[pair_index];
                const int k=rotor ? ((round+stage+pair_index)&3) : 0;
                add_cell(out,left,right,k);
            }
        }
    } else if (topology=="chain") {
        for (int stage=0;stage<15;++stage) {
            const int k=rotor ? ((round+stage)&3) : 0;
            add_cell(out,stage,stage+1,k);
        }
    } else if (topology=="ring") {
        for (int stage=0;stage<16;++stage) {
            const int k=rotor ? ((round+stage)&3) : 0;
            add_cell(out,stage,(stage+1)&15,k);
        }
    } else {
        throw std::runtime_error("topology must be chain, ring, butterfly3, or butterfly4");
    }
    return out;
}
static State apply_layer(
    State x, const std::string& topology, int round, bool rotor, bool inverse=false) {
    const auto steps=layer_steps(topology,round,rotor);
    if (!inverse) {
        for (const auto& s:steps)
            x[s.target]^=apply_matrix8(MATRIX_FAMILY[s.orientation],x[s.source]);
    } else {
        for (auto it=steps.rbegin();it!=steps.rend();++it)
            x[it->target]^=apply_matrix8(MATRIX_FAMILY[it->orientation],x[it->source]);
    }
    return x;
}
static U128 state_u128(const State& x) {
    U128 v=0; for (auto b:x) v=(v<<8)|b; return v;
}
static std::array<U128,128> columns(
    const std::string& topology, int round, bool rotor, bool inverse=false) {
    std::array<U128,128> out{};
    for (int bit=0;bit<128;++bit) {
        State x{}; x[bit/8]=static_cast<std::uint8_t>(1u<<(7-(bit%8)));
        out[bit]=state_u128(apply_layer(x,topology,round,rotor,inverse));
    }
    return out;
}
static U128 byte_mask(int b) {
    return static_cast<U128>(0xFF)<<(8*(15-b));
}
static int top_bit(U128 x) {
    const std::uint64_t hi=static_cast<std::uint64_t>(x>>64);
    if (hi) return 64+63-__builtin_clzll(hi);
    const std::uint64_t lo=static_cast<std::uint64_t>(x);
    return lo ? 63-__builtin_clzll(lo) : -1;
}
static int rank_vectors(const std::vector<U128>& vectors) {
    U128 basis[128]={}; int rank=0;
    for (auto x:vectors) while (x) {
        const int bit=top_bit(x);
        if (basis[bit]) x^=basis[bit];
        else { basis[bit]=x; ++rank; break; }
    }
    return rank;
}
static bool independent(const std::vector<U128>& vectors) {
    return rank_vectors(vectors)==static_cast<int>(vectors.size());
}
static SearchResult search_case(
    const std::array<U128,128>& C, int input_bytes, int output_bytes, bool inverse) {
    SearchResult result;
    result.input_bytes=input_bytes; result.output_bytes=output_bytes; result.inverse=inverse;
    for (auto I:COMBINATIONS[input_bytes]) {
        std::vector<U128> selected;
        for (int b=0;b<16;++b) if ((I>>b)&1u)
            for (int bit=0;bit<8;++bit) selected.push_back(C[8*b+bit]);
        for (auto J:COMBINATIONS[output_bytes]) {
            const U128 mask=FORBIDDEN_MASKS[J];
            std::vector<U128> projected;
            for (auto v:selected) projected.push_back(v&mask);
            ++result.checks;
            if (!independent(projected)) { result.found=true; return result; }
        }
    }
    return result;
}
static int active_bytes(const State& x) {
    int n=0; for (auto b:x) n+=(b!=0); return n;
}
static std::pair<int,int> one_active_range(
    const std::string& topology, int round, bool rotor) {
    int minimum=16, maximum=0;
    for (int byte=0;byte<16;++byte) for (int value=1;value<256;++value) {
        State x{}; x[byte]=static_cast<std::uint8_t>(value);
        const int w=active_bytes(apply_layer(x,topology,round,rotor));
        minimum=std::min(minimum,w); maximum=std::max(maximum,w);
    }
    return {minimum,maximum};
}
static std::pair<int,int> byte_block_rank_range(const std::array<U128,128>& C) {
    int minimum=8, maximum=0;
    for (int output_byte=0;output_byte<16;++output_byte) {
        const U128 mask=byte_mask(output_byte);
        for (int input_byte=0;input_byte<16;++input_byte) {
            std::vector<U128> block;
            for (int bit=0;bit<8;++bit) block.push_back(C[8*input_byte+bit]&mask);
            const int rank=rank_vectors(block);
            minimum=std::min(minimum,rank); maximum=std::max(maximum,rank);
        }
    }
    return {minimum,maximum};
}
static void initialize() {
    MATRIX_FAMILY[0]=BASE_MATRIX;
    for (int k=1;k<4;++k) MATRIX_FAMILY[k]=rotate_clockwise(MATRIX_FAMILY[k-1]);
    for (std::uint32_t mask=0;mask<(1u<<16);++mask)
        COMBINATIONS[__builtin_popcount(mask)].push_back(static_cast<std::uint16_t>(mask));
    const U128 all=~static_cast<U128>(0);
    for (std::uint32_t J=0;J<(1u<<16);++J) {
        U128 forbidden=all;
        for (int b=0;b<16;++b) if ((J>>b)&1u) forbidden^=byte_mask(b);
        FORBIDDEN_MASKS[J]=forbidden;
    }
}

int main(int argc,char** argv) {
    initialize();
    const std::string topology=argc>=2 ? argv[1] : "butterfly4";
    const std::string schedule=argc>=3 ? argv[2] : "static";
    const bool rotor=schedule=="rotor";
    if (schedule!="static" && schedule!="rotor") {
        std::cerr<<"schedule must be static or rotor\n"; return 2;
    }
    const int round=argc>=4 ? std::stoi(argv[3]) : 0;
    const auto C=columns(topology,round,rotor,false);
    const auto CI=columns(topology,round,rotor,true);
    const auto one=one_active_range(topology,round,rotor);
    const auto block=byte_block_rank_range(C);
    const auto step_list=layer_steps(topology,round,rotor);

    int branch=0, split_a=0, split_b=0;
    for (int sum=2;sum<=17 && branch==0;++sum) {
        for (int t=1;t<=sum/2 && branch==0;++t) {
            const int u=sum-t; if (u>16) continue;
            auto F=search_case(C,t,u,false);
            if (F.found) { branch=sum; split_a=t; split_b=u; break; }
            if (t<u) {
                auto G=search_case(CI,t,u,true);
                if (G.found) { branch=sum; split_a=u; split_b=t; break; }
            }
        }
    }

    std::cout<<"topology="<<topology
             <<" schedule="<<schedule
             <<" round="<<round
             <<" cells="<<(step_list.size()/3)
             <<" shears="<<step_list.size()
             <<" full_rank="<<rank_vectors(std::vector<U128>(C.begin(),C.end()))
             <<" one_active_output="<<one.first<<".."<<one.second
             <<" byte_block_rank="<<block.first<<".."<<block.second
             <<" byte_branch_number="<<branch
             <<" witness_split="<<split_a<<"->"<<split_b
             <<"\n";
    return 0;
}

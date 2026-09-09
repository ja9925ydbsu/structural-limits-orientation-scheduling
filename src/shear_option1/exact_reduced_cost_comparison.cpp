// Exact reduced-cost butterfly comparison for Option 1.
//
// Family: three complete butterfly stages (masks 1,2,4), followed by a
// selectable subset of the eight mask-8 cells. A bit set in final_keep_mask
// retains final-stage pair (i, i^8) for i=0..7.
//
// Examples:
//   0x00 : 24 cells / 72 shears (three-stage butterfly)
//   0x6f : 30 cells / 90 shears (selected B=7 reduced control)
//   0xff : 32 cells / 96 shears (full four-stage reference)
//
// Branch numbers are determined by exhaustive support-set GF(2) rank tests.
// This is an Option 1 architectural-development tool only.

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

using U128 = __uint128_t;
using Matrix8 = std::array<std::uint8_t, 8>;
using State = std::array<std::uint8_t, 16>;

static const Matrix8 BASE_MATRIX = {
    0xC8, 0x73, 0xF5, 0x14, 0x01, 0xC0, 0xDE, 0x82
};
static std::array<Matrix8, 4> MATRIX_FAMILY;
static std::array<std::vector<std::uint16_t>, 17> COMBINATIONS;
static std::array<U128, 1u << 16> FORBIDDEN_MASKS;

struct Step { int target, source, orientation; };

static int parity8(std::uint8_t x) {
    return __builtin_popcount(static_cast<unsigned>(x)) & 1;
}
static std::uint8_t apply_matrix8(const Matrix8& m, std::uint8_t x) {
    std::uint8_t y = 0;
    for (int i = 0; i < 8; ++i)
        y |= static_cast<std::uint8_t>(parity8(m[i] & x) << (7 - i));
    return y;
}
static Matrix8 rotate_clockwise(const Matrix8& m) {
    int a[8][8] = {}, b[8][8] = {};
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            a[r][c] = (m[r] >> (7 - c)) & 1;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            b[r][c] = a[7 - c][r];
    Matrix8 out{};
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            out[r] |= static_cast<std::uint8_t>(b[r][c] << (7 - c));
    return out;
}
static std::vector<std::pair<int,int>> butterfly_pairs(int mask) {
    std::vector<std::pair<int,int>> out;
    for (int i = 0; i < 16; ++i)
        if (i < (i ^ mask)) out.push_back({i, i ^ mask});
    return out;
}
static void add_cell(std::vector<Step>& out, int left, int right, int k) {
    out.push_back({left, right, k});
    out.push_back({right, left, k ^ 1});
    out.push_back({left, right, k});
}
static std::vector<Step> layer_steps(
    int round, bool rotor, std::uint8_t final_keep_mask) {
    std::vector<Step> out;
    const int masks[4] = {1, 2, 4, 8};
    for (int stage = 0; stage < 4; ++stage) {
        const auto pairs = butterfly_pairs(masks[stage]);
        for (int pair_index = 0; pair_index < 8; ++pair_index) {
            if (stage == 3 && ((final_keep_mask >> pair_index) & 1u) == 0)
                continue;
            const auto [left, right] = pairs[pair_index];
            const int k = rotor ? ((round + stage + pair_index) & 3) : 0;
            add_cell(out, left, right, k);
        }
    }
    return out;
}
static State apply_layer(
    State x, int round, bool rotor, std::uint8_t final_keep_mask,
    bool inverse = false) {
    const auto steps = layer_steps(round, rotor, final_keep_mask);
    if (!inverse) {
        for (const auto& s : steps)
            x[s.target] ^= apply_matrix8(MATRIX_FAMILY[s.orientation], x[s.source]);
    } else {
        for (auto it = steps.rbegin(); it != steps.rend(); ++it)
            x[it->target] ^= apply_matrix8(MATRIX_FAMILY[it->orientation], x[it->source]);
    }
    return x;
}
static U128 state_u128(const State& x) {
    U128 v = 0;
    for (auto b : x) v = (v << 8) | b;
    return v;
}
static std::array<U128,128> columns(
    int round, bool rotor, std::uint8_t final_keep_mask, bool inverse = false) {
    std::array<U128,128> out{};
    for (int bit = 0; bit < 128; ++bit) {
        State x{};
        x[bit / 8] = static_cast<std::uint8_t>(1u << (7 - (bit % 8)));
        out[bit] = state_u128(apply_layer(x, round, rotor, final_keep_mask, inverse));
    }
    return out;
}
static U128 byte_mask(int b) {
    return static_cast<U128>(0xFF) << (8 * (15 - b));
}
static int top_bit(U128 x) {
    const std::uint64_t hi = static_cast<std::uint64_t>(x >> 64);
    if (hi) return 64 + 63 - __builtin_clzll(hi);
    const std::uint64_t lo = static_cast<std::uint64_t>(x);
    return lo ? 63 - __builtin_clzll(lo) : -1;
}
static int rank_vectors(const std::vector<U128>& vectors) {
    U128 basis[128] = {};
    int rank = 0;
    for (auto x : vectors) {
        while (x) {
            const int bit = top_bit(x);
            if (basis[bit]) x ^= basis[bit];
            else { basis[bit] = x; ++rank; break; }
        }
    }
    return rank;
}
static bool independent(const std::vector<U128>& vectors) {
    return rank_vectors(vectors) == static_cast<int>(vectors.size());
}
static bool support_feasible(
    const std::array<U128,128>& C, int input_bytes, int output_bytes) {
    for (auto I : COMBINATIONS[input_bytes]) {
        std::vector<U128> selected;
        selected.reserve(8 * input_bytes);
        for (int b = 0; b < 16; ++b)
            if ((I >> b) & 1u)
                for (int bit = 0; bit < 8; ++bit)
                    selected.push_back(C[8 * b + bit]);
        for (auto J : COMBINATIONS[output_bytes]) {
            const U128 mask = FORBIDDEN_MASKS[J];
            std::vector<U128> projected;
            projected.reserve(selected.size());
            for (auto v : selected) projected.push_back(v & mask);
            if (!independent(projected)) return true;
        }
    }
    return false;
}
static int branch_number(
    const std::array<U128,128>& C, const std::array<U128,128>& CI,
    int& split_in, int& split_out) {
    for (int sum = 2; sum <= 17; ++sum) {
        for (int t = 1; t <= sum / 2; ++t) {
            const int u = sum - t;
            if (u > 16) continue;
            if (support_feasible(C, t, u)) {
                split_in = t; split_out = u; return sum;
            }
            if (t < u && support_feasible(CI, t, u)) {
                split_in = u; split_out = t; return sum;
            }
        }
    }
    return 0;
}
static int active_bytes(const State& x) {
    int n = 0;
    for (auto b : x) n += (b != 0);
    return n;
}
static std::pair<int,int> one_active_range(
    int round, bool rotor, std::uint8_t final_keep_mask) {
    int minimum = 16, maximum = 0;
    for (int byte = 0; byte < 16; ++byte) {
        for (int value = 1; value < 256; ++value) {
            State x{};
            x[byte] = static_cast<std::uint8_t>(value);
            const int w = active_bytes(apply_layer(x, round, rotor, final_keep_mask));
            minimum = std::min(minimum, w);
            maximum = std::max(maximum, w);
        }
    }
    return {minimum, maximum};
}
static std::pair<int,int> byte_block_rank_range(const std::array<U128,128>& C) {
    int minimum = 8, maximum = 0;
    for (int output_byte = 0; output_byte < 16; ++output_byte) {
        const U128 mask = byte_mask(output_byte);
        for (int input_byte = 0; input_byte < 16; ++input_byte) {
            std::vector<U128> block;
            for (int bit = 0; bit < 8; ++bit)
                block.push_back(C[8 * input_byte + bit] & mask);
            const int rank = rank_vectors(block);
            minimum = std::min(minimum, rank);
            maximum = std::max(maximum, rank);
        }
    }
    return {minimum, maximum};
}
static void initialize() {
    MATRIX_FAMILY[0] = BASE_MATRIX;
    for (int k = 1; k < 4; ++k)
        MATRIX_FAMILY[k] = rotate_clockwise(MATRIX_FAMILY[k - 1]);
    for (std::uint32_t mask = 0; mask < (1u << 16); ++mask)
        COMBINATIONS[__builtin_popcount(mask)].push_back(static_cast<std::uint16_t>(mask));
    const U128 all = ~static_cast<U128>(0);
    for (std::uint32_t J = 0; J < (1u << 16); ++J) {
        U128 forbidden = all;
        for (int b = 0; b < 16; ++b)
            if ((J >> b) & 1u) forbidden ^= byte_mask(b);
        FORBIDDEN_MASKS[J] = forbidden;
    }
}
static std::uint8_t parse_mask(const std::string& s) {
    const unsigned long value = std::stoul(s, nullptr, 0);
    if (value > 0xFFul) throw std::runtime_error("final_keep_mask must be in 0..255");
    return static_cast<std::uint8_t>(value);
}

int main(int argc, char** argv) {
    initialize();
    const std::string schedule = argc >= 2 ? argv[1] : "static";
    if (schedule != "static" && schedule != "rotor") {
        std::cerr << "usage: " << argv[0]
                  << " [static|rotor] [round] [final_keep_mask]\n";
        return 2;
    }
    const bool rotor = schedule == "rotor";
    const int round = argc >= 3 ? std::stoi(argv[2]) : 0;
    const std::uint8_t keep = argc >= 4 ? parse_mask(argv[3]) : 0xFF;

    const auto C = columns(round, rotor, keep, false);
    const auto CI = columns(round, rotor, keep, true);
    const auto one = one_active_range(round, rotor, keep);
    const auto blocks = byte_block_rank_range(C);
    int split_in = 0, split_out = 0;
    const int branch = branch_number(C, CI, split_in, split_out);
    const int cells = 24 + __builtin_popcount(static_cast<unsigned>(keep));

    std::cout << "schedule=" << schedule
              << " round=" << round
              << " final_keep_mask=0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<unsigned>(keep) << std::dec
              << " cells=" << cells
              << " shears=" << (3 * cells)
              << " full_rank=" << rank_vectors(std::vector<U128>(C.begin(), C.end()))
              << " one_active_output=" << one.first << ".." << one.second
              << " byte_block_rank=" << blocks.first << ".." << blocks.second
              << " byte_branch_number=" << branch
              << " witness_split=" << split_in << "->" << split_out
              << "\n";
    return 0;
}

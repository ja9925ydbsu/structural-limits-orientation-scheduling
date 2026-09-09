// Exact byte-level branch-number checker for Option 1.
//
// This program is self-contained and rebuilds the same deterministic 8x8
// matrix family and four-stage butterfly shear layer as option1_shear_core.py.
// It uses exhaustive support-set/rank tests over GF(2); there is no random
// sampling and no external MILP/SAT dependency.
//
// For candidate input-byte support I and allowed output-byte support J, a
// nonzero input supported in I can map entirely into J iff
//
//   L[output bytes outside J, input bits inside I]
//
// has a nontrivial nullspace.  Cases with |I|>|J| are checked through L^-1,
// using B(L)=B(L^-1).

#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using U128 = __uint128_t;
using Matrix8 = std::array<std::uint8_t, 8>;
using State = std::array<std::uint8_t, 16>;

static const Matrix8 BASE_MATRIX = {
    0xC8, 0x73, 0xF5, 0x14, 0x01, 0xC0, 0xDE, 0x82
};
static std::array<Matrix8, 4> MATRIX_FAMILY;
static std::array<std::vector<std::uint16_t>, 17> COMBINATIONS;
static std::array<U128, 1u << 16> FORBIDDEN_MASKS;

struct ShearStep { int target, source, orientation; };
struct Result {
    bool found = false;
    int in_bytes = 0, out_bytes = 0;
    std::uint16_t in_support = 0, out_support = 0;
    std::uint64_t relation = 0, checks = 0;
    bool inverse = false;
};

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
    for (int r = 0; r < 8; ++r) {
        int v = 0;
        for (int c = 0; c < 8; ++c) v |= b[r][c] << (7 - c);
        out[r] = static_cast<std::uint8_t>(v);
    }
    return out;
}
static std::vector<std::pair<int,int>> butterfly_pairs(int mask) {
    std::vector<std::pair<int,int>> out;
    for (int i = 0; i < 16; ++i) if (i < (i ^ mask)) out.push_back({i, i ^ mask});
    return out;
}
static std::vector<ShearStep> layer_steps(int round, bool rotor) {
    std::vector<ShearStep> out;
    const int masks[4] = {1,2,4,8};
    for (int stage = 0; stage < 4; ++stage) {
        const auto pairs = butterfly_pairs(masks[stage]);
        for (int pi = 0; pi < static_cast<int>(pairs.size()); ++pi) {
            const auto [left,right] = pairs[pi];
            const int k = rotor ? ((round + stage + pi) & 3) : 0;
            out.push_back({left,right,k});
            out.push_back({right,left,k ^ 1});
            out.push_back({left,right,k});
        }
    }
    return out;
}
static State apply_layer(State x, int round, bool rotor, bool inverse=false) {
    const auto steps = layer_steps(round, rotor);
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
static std::array<U128,128> columns(int round, bool rotor, bool inverse=false) {
    std::array<U128,128> out{};
    for (int j = 0; j < 128; ++j) {
        State x{};
        x[j/8] = static_cast<std::uint8_t>(1u << (7 - (j % 8)));
        out[j] = state_u128(apply_layer(x, round, rotor, inverse));
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
static bool independent(const std::vector<U128>& vectors) {
    U128 basis[128] = {};
    for (auto x : vectors) {
        while (x) {
            const int b = top_bit(x);
            if (basis[b]) x ^= basis[b];
            else { basis[b] = x; break; }
        }
        if (!x) return false;
    }
    return true;
}
static std::uint64_t null_relation(const std::vector<U128>& vectors) {
    U128 basis[128] = {};
    std::uint64_t coeff[128] = {};
    for (int j = 0; j < static_cast<int>(vectors.size()); ++j) {
        U128 x = vectors[j];
        std::uint64_t c = 1ULL << j;
        while (x) {
            const int b = top_bit(x);
            if (basis[b]) { x ^= basis[b]; c ^= coeff[b]; }
            else { basis[b] = x; coeff[b] = c; break; }
        }
        if (!x) return c;
    }
    return 0;
}
static Result search_case(const std::array<U128,128>& C, int t, int u, bool inverse) {
    Result R;
    R.in_bytes = t; R.out_bytes = u; R.inverse = inverse;
    for (auto I : COMBINATIONS[t]) {
        std::vector<U128> selected;
        selected.reserve(8*t);
        for (int b = 0; b < 16; ++b) if ((I >> b) & 1u)
            for (int bit = 0; bit < 8; ++bit) selected.push_back(C[8*b + bit]);
        for (auto J : COMBINATIONS[u]) {
            const U128 mask = FORBIDDEN_MASKS[J];
            std::vector<U128> projected;
            projected.reserve(selected.size());
            for (auto v : selected) projected.push_back(v & mask);
            ++R.checks;
            if (!independent(projected)) {
                R.found = true; R.in_support = I; R.out_support = J;
                R.relation = null_relation(projected);
                return R;
            }
        }
    }
    return R;
}
static State relation_input(std::uint16_t support, std::uint64_t relation) {
    State x{};
    int pos = 0;
    for (int b = 0; b < 16; ++b) if ((support >> b) & 1u)
        for (int bit = 0; bit < 8; ++bit) {
            if ((relation >> pos) & 1ULL) x[b] |= static_cast<std::uint8_t>(1u << (7-bit));
            ++pos;
        }
    return x;
}
static int active_bytes(const State& x) {
    int n = 0; for (auto b : x) n += (b != 0); return n;
}
static std::string hex_state(const State& x) {
    std::ostringstream out; out << std::hex << std::setfill('0');
    for (auto b : x) out << std::setw(2) << static_cast<int>(b);
    return out.str();
}
static int rank_vectors(const std::vector<U128>& vectors) {
    U128 basis[128] = {}; int rank = 0;
    for (auto x : vectors) while (x) {
        const int b = top_bit(x);
        if (basis[b]) x ^= basis[b];
        else { basis[b] = x; ++rank; break; }
    }
    return rank;
}
static int full_rank(const std::array<U128,128>& C) {
    return rank_vectors(std::vector<U128>(C.begin(), C.end()));
}
static void initialize() {
    MATRIX_FAMILY[0] = BASE_MATRIX;
    for (int k = 1; k < 4; ++k) MATRIX_FAMILY[k] = rotate_clockwise(MATRIX_FAMILY[k-1]);
    for (std::uint32_t m = 0; m < (1u << 16); ++m)
        COMBINATIONS[__builtin_popcount(m)].push_back(static_cast<std::uint16_t>(m));
    const U128 all = ~static_cast<U128>(0);
    for (std::uint32_t J = 0; J < (1u << 16); ++J) {
        U128 forbidden = all;
        for (int b = 0; b < 16; ++b) if ((J >> b) & 1u) forbidden ^= byte_mask(b);
        FORBIDDEN_MASKS[J] = forbidden;
    }
}

int main(int argc, char** argv) {
    initialize();
    const std::string schedule = argc >= 2 ? argv[1] : "static";
    if (schedule != "static" && schedule != "rotor") {
        std::cerr << "usage: " << argv[0] << " [static|rotor] [round]\n";
        return 2;
    }
    const bool rotor = schedule == "rotor";
    const int round = argc >= 3 ? std::stoi(argv[2]) : 0;
    const auto C = columns(round, rotor, false);
    const auto CI = columns(round, rotor, true);
    std::cout << "schedule=" << schedule << " round=" << round
              << " full_layer_rank=" << full_rank(C) << "\n";

    Result witness;
    for (int sum = 2; sum <= 17; ++sum) {
        const auto start = std::chrono::steady_clock::now();
        std::uint64_t checks = 0;
        bool found = false;
        for (int t = 1; t <= sum/2; ++t) {
            const int u = sum - t;
            if (u > 16) continue;
            auto F = search_case(C, t, u, false);
            checks += F.checks;
            if (F.found) { witness = F; found = true; break; }
            if (t < u) {
                auto G = search_case(CI, t, u, true);
                checks += G.checks;
                if (G.found) { witness = G; found = true; break; }
            }
        }
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start).count();
        std::cout << "sum=" << sum << " checks=" << checks
                  << " seconds=" << std::fixed << std::setprecision(6) << seconds
                  << " result=" << (found ? "feasible" : "excluded") << "\n";
        if (found) {
            std::cout << "byte_branch_number=" << sum << "\n";
            break;
        }
    }

    if (!witness.found) return 1;
    State a = relation_input(witness.in_support, witness.relation);
    State b = apply_layer(a, round, rotor, witness.inverse);
    State forward_in = witness.inverse ? b : a;
    State forward_out = witness.inverse ? a : b;
    std::cout << "witness_input=" << hex_state(forward_in)
              << " input_weight=" << active_bytes(forward_in) << "\n";
    std::cout << "witness_output=" << hex_state(forward_out)
              << " output_weight=" << active_bytes(forward_out) << "\n";
    return 0;
}

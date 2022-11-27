//
//  bitboard.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#pragma once
#ifndef bitboard_h
#define bitboard_h

#include "fruit.h"
#include "definitions.h"

namespace Bitboards
{

constexpr Bitboard RANK_1 = 0xffULL;
constexpr Bitboard RANK_2 = RANK_1 << 8;
constexpr Bitboard RANK_3 = RANK_1 << 16;
constexpr Bitboard RANK_4 = RANK_1 << 24;
constexpr Bitboard RANK_5 = RANK_1 << 32;
constexpr Bitboard RANK_6 = RANK_1 << 40;
constexpr Bitboard RANK_7 = RANK_1 << 48;
constexpr Bitboard RANK_8 = RANK_1 << 56;

constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_B = FILE_A << 1;
constexpr Bitboard FILE_C = FILE_A << 2;
constexpr Bitboard FILE_D = FILE_A << 3;
constexpr Bitboard FILE_E = FILE_A << 4;
constexpr Bitboard FILE_F = FILE_A << 5;
constexpr Bitboard FILE_G = FILE_A << 6;
constexpr Bitboard FILE_H = FILE_A << 7;

void init();

} // namespace Bitboards

extern Bitboard BITBOARD_FOR_SQUARE[64];
extern Bitboard KNIGHT_SPAN[64];
extern Bitboard ADJACENT_SQUARES[64];
extern Bitboard RANKS[64];
extern Bitboard FILES[64];
extern Bitboard DIAGONALS[64];
extern Bitboard ANTI_DIAGONALS[64];

inline Bitboard square_to_bitboard(int square)
{
	return BITBOARD_FOR_SQUARE[square];
}

inline int popcount(Bitboard B)
{
	return __builtin_popcountll(B);
}

template<Direction D>
constexpr Bitboard shift(Bitboard B)
{
	if (D == NORTH) return B << 8;
	if (D == NORTH_EAST) return (B << 9) & ~Bitboards::FILE_A;
	if (D == EAST) return (B << 1) & ~Bitboards::FILE_A;
	if (D == SOUTH_EAST) return (B >> 7) & ~Bitboards::FILE_A;
	if (D == SOUTH) return B >> 8;
	if (D == SOUTH_WEST) return (B >> 9) & ~Bitboards::FILE_H;
	if (D == WEST) return (B >> 1) & ~Bitboards::FILE_H;
	if (D == NORTH_WEST) return (B << 7) & ~Bitboards::FILE_H;
	return 0;
}

//static const uint64_t reversed_double_bytes[65536] =
//{
//	#define two14 (1 << 14)
//	#define two12 (1 << 12)
//	#define two10 (1 << 10)
//	#define two8  (1 << 8)
//	#define two6  (1 << 6)
//	#define two4  (1 << 4)
//	#define two2  (1 << 2)
//	#define R2(n) n,       n + 2 * two14,     n + 1 * two14,     n + 3 * two14
//	#define R4(n) R2(n),   R2(n + 2 * two12), R2(n + 1 * two12), R2(n + 3 * two12)
//	#define R6(n) R4(n),   R4(n + 2 * two10), R4(n + 1 * two10), R4(n + 3 * two10)
//	#define R8(n) R6(n),   R6(n + 2 * two8),  R6(n + 1 * two8),  R6(n + 3 * two8)
//	#define R10(n) R8(n),  R8(n + 2 * two6),  R8(n + 1 * two6),  R8(n + 3 * two6)
//	#define R12(n) R10(n), R10(n + 2 * two4), R10(n + 1 * two4), R10(n + 3 * two4)
//	#define R14(n) R12(n), R12(n + 2 * two2), R12(n + 1 * two2), R12(n + 3 * two2)
//	R14(0), R14(2), R14(1), R14(3)
//	#undef two14
//	#undef two12
//	#undef two10
//	#undef two8
//	#undef two6
//	#undef two4
//	#undef two2
//	#undef R2
//	#undef R4
//	#undef R6
//	#undef R8
//	#undef R10
//	#undef R12
//	#undef R14
//};
//inline Bitboard reverse(Bitboard B)
//{
//	return
//		(reversed_double_bytes[B & 0xffffULL] << 48) |
//		(reversed_double_bytes[(B >> 16) & 0xffffULL] << 32) |
//		(reversed_double_bytes[(B >> 32) & 0xffffULL] << 16) |
//		(reversed_double_bytes[(B >> 48) & 0xffffULL]);
//}
static const uint64_t reversed_double_bytes[256] =
{
	#define two6  (1 << 6)
	#define two4  (1 << 4)
	#define two2  (1 << 2)
	#define R2(n) n,       n + 2 * two6,     n + 1 * two6,     n + 3 * two6
	#define R4(n) R2(n),   R2(n + 2 * two4), R2(n + 1 * two4), R2(n + 3 * two4)
	#define R6(n) R4(n),   R4(n + 2 * two2), R4(n + 1 * two2), R4(n + 3 * two2)
	R6(0), R6(2), R6(1), R6(3)
	#undef two6
	#undef two4
	#undef two2
	#undef R2
	#undef R4
	#undef R6
};
inline Bitboard reverse(Bitboard B)
{
	return
		(reversed_double_bytes[B & 0xffULL] << 56) |
		(reversed_double_bytes[(B >> 8) & 0xffULL] << 48) |
		(reversed_double_bytes[(B >> 16) & 0xffULL] << 40) |
		(reversed_double_bytes[(B >> 24) & 0xffULL] << 32) |
		(reversed_double_bytes[(B >> 32) & 0xffULL] << 24) |
		(reversed_double_bytes[(B >> 40) & 0xffULL] << 16) |
		(reversed_double_bytes[(B >> 48) & 0xffULL] << 8) |
		(reversed_double_bytes[(B >> 56) & 0xffULL]);
}

std::string visual(Bitboard B);

inline int lsb(Bitboard B)
{
	return __builtin_ctzll(B);
}
inline int pop_lsb(Bitboard &B)
{
	const int square = lsb(B);
	B &= B - 1;
	return square;
}
inline int msb(Bitboard B)
{
	return 63 - __builtin_clzll(B);
}
/// Undefined behavior if `B == 0`.
inline int pop_msb(Bitboard &B)
{
	const int square = msb(B);
	B ^= square_to_bitboard(square);
	return square;
}

#endif /* bitboard_h */

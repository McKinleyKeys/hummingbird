//
//  bitboard.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#include "bitboard.h"

Bitboard BITBOARD_FOR_SQUARE[64];
Bitboard KNIGHT_SPAN[64];
Bitboard ADJACENT_SQUARES[64];
Bitboard RANKS[64];
Bitboard FILES[64];
Bitboard DIAGONALS[64];
Bitboard ANTI_DIAGONALS[64];

void Bitboards::init()
{
	for (int square = 0; square < 64; square++)
		BITBOARD_FOR_SQUARE[square] = 1ULL << square;
	
	// Knight span
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			Bitboard SPAN = 0;
			for (int dx : {-2, -1, +1, +2}) {
				for (int dy : {-2, -1, +1, +2}) {
					if (abs(dx) + abs(dy) != 3) continue;
					// Check that this square is on the board
					if (x + dx >= 0 && x + dx < 8 &&
						y + dy >= 0 && y + dy < 8) {
						int square = 8 * (y + dy) + x + dx;
						SPAN |= square_to_bitboard(square);
					}
				}
			}
			KNIGHT_SPAN[8 * y + x] = SPAN;
		}
	}
	
	// King span
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			Bitboard SPAN = 0;
			for (int dx = -1; dx <= 1; dx++) {
				for (int dy = -1; dy <= 1; dy++) {
					if (dx == 0 && dy == 0) continue;
					// Check that this square is on the board
					if (x + dx >= 0 && x + dx < 8 &&
						y + dy >= 0 && y + dy < 8) {
						int square = 8 * (y + dy) + x + dx;
						SPAN |= square_to_bitboard(square);
					}
				}
			}
			ADJACENT_SQUARES[8 * y + x] = SPAN;
		}
	}
	
	// Ranks
	for (int square = 0; square < 64; square++) {
		int y = square / 8;
		Bitboard RANK = 0xffULL << (y * 8);
		RANKS[square] = RANK;
	}
	
	// Files
	for (int square = 0; square < 64; square++) {
		int x = square % 8;
		Bitboard FILE = 0x0101010101010101ULL << x;
		FILES[square] = FILE;
	}
	
	// Diagonals
	for (int square = 0; square < 64; square++) {
		int y = square / 8, x = square % 8;
		Bitboard DIAGONAL = 0;
		for (int delta = -std::min(y, x); delta <= std::min(7 - y, 7 - x); delta++) {
			int other = 8 * (y + delta) + x + delta;
			DIAGONAL |= 1ULL << other;
		}
		DIAGONALS[square] = DIAGONAL;
	}
	
	// Anti-diagonals
	for (int square = 0; square < 64; square++) {
		int y = square / 8, x = square % 8;
		Bitboard ANTI_DIAGONAL = 0;
		for (int delta = -std::min(y, 7 - x); delta <= std::min(7 - y, x); delta++) {
			int other = 8 * (y + delta) + x - delta;
			ANTI_DIAGONAL |= 1ULL << other;
		}
		ANTI_DIAGONALS[square] = ANTI_DIAGONAL;
	}
}

std::string visual(Bitboard B)
{
	std::string s = "";
	for (int rank = 8; rank --> 0;) {
		for (int file = 0; file < 8; file++) {
			s += B & (1ULL << (8 * rank + file)) ? "1" : ".";
			if (file < 7)
				s += " ";
		}
		if (rank > 0)
			s += "\n";
	}
	return s;
}

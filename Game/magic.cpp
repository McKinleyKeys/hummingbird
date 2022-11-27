//
//  magic.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#include "magic.h"
#include "notation.h"

namespace Magic
{

Bitboard RING_OF_RADIUS_2;
Bitboard RING_OF_RADIUS_3;
Bitboard EDGE_SQUARES;

int START_OF_RANK[64];
Bitboard HORIZONTAL_SPAN[64][256];

Bitboard VERTICAL_MULTIPLICAND[64];
Bitboard VERTICAL_SPAN[64][256];

Bitboard DIAGONAL_MULTIPLICAND[64];
Bitboard DIAGONAL_SPAN[64][256];

Bitboard ANTI_DIAGONAL_MULTIPLICAND[64];
Bitboard ANTI_DIAGONAL_SPAN[64][256];

int PIECE_SCORES[2][2][PIECE_COUNT][64];


void init()
{
	// Initialize `RING_OF_RADIUS_2`
	RING_OF_RADIUS_2 |= 15ULL << C3;
	RING_OF_RADIUS_2 |= square_to_bitboard(C4) | square_to_bitboard(C5);
	RING_OF_RADIUS_2 |= square_to_bitboard(F4) | square_to_bitboard(F5);
	RING_OF_RADIUS_2 |= 15ULL << C6;
	
	// Initialize `RING_OF_RADIUS_3`
	RING_OF_RADIUS_3 |= 63ULL << B2;
	RING_OF_RADIUS_3 |= square_to_bitboard(B3) | square_to_bitboard(B4) | square_to_bitboard(B5) | square_to_bitboard(B6);
	RING_OF_RADIUS_3 |= square_to_bitboard(G3) | square_to_bitboard(G4) | square_to_bitboard(G5) | square_to_bitboard(G6);
	RING_OF_RADIUS_3 |= 63ULL << B7;
	
	// Initialize `EDGE_SQUARES`
	EDGE_SQUARES = ~0 & ~(CENTER_FOUR_SQUARES | RING_OF_RADIUS_2 | RING_OF_RADIUS_3);
	
	// Initialize `START_OF_RANK`
	for (int square = 0; square < 64; square++) {
		const int y = square / 8;
		START_OF_RANK[square] = 8 * y;
	}
	
	// Initialize `HORIZONTAL_SPAN`
	for (int square = 0; square < 64; square++) {
		const int y = square / 8, x = square % 8;
		const Bitboard S = square_to_bitboard(x);
		const Bitboard S_ = reverse(S);
		for (Bitboard O = 0; O < 256; O++) {
			
			Bitboard SPAN = 0;
			// Right span
			SPAN |= O ^ (O - 2 * S);
			// Left span
			SPAN |= (O ^ reverse(reverse(O) - 2 * S_));
			
			SPAN &= Bitboards::RANK_1;
			
			HORIZONTAL_SPAN[square][O] = SPAN << (8 * y);
		}
	}
	
	// Initialize `VERTICAL_MULTIPLICAND`
	for (int square = 0; square < 64; square++) {
		Bitboard M = 0;
		Bitboard FILE = FILES[square];
		int i = 0;
		while (FILE) {
			const Bitboard ORIGIN = square_to_bitboard(pop_lsb(FILE));
			const Bitboard DESTINATION = square_to_bitboard(A8 + i);
			M |= DESTINATION / ORIGIN;
			i++;
		}
		VERTICAL_MULTIPLICAND[square] = M;
	}
	
	// Initialize `VERTICAL_SPAN`
	for (int square = 0; square < 64; square++) {
		for (Bitboard O = 0; O < 256; O++) {
			
			// Figure out the index of `square` in its file
			const int index = square / 8;
			
			const Bitboard SPAN = HORIZONTAL_SPAN[index][O];
			
			// Rotate `SPAN` back to the file
			Bitboard rSPAN = 0;
			Bitboard FILE = FILES[square];
			int i = 0;
			while (FILE) {
				const int file_square = pop_lsb(FILE);
				if (SPAN & square_to_bitboard(i))
					rSPAN |= square_to_bitboard(file_square);
				i++;
			}
			
			VERTICAL_SPAN[square][O] = rSPAN;
		}
	}
	
	// Initialize `DIAGONAL_MULTIPLICAND`
	for (int square = 0; square < 64; square++) {
		Bitboard M = 0;
		Bitboard DIAGONAL = DIAGONALS[square];
		int i = 0;
		while (DIAGONAL) {
			const Bitboard ORIGIN = square_to_bitboard(pop_lsb(DIAGONAL));
			const Bitboard DESTINATION = square_to_bitboard(A8 + i);
			M |= DESTINATION / ORIGIN;
			i++;
		}
		DIAGONAL_MULTIPLICAND[square] = M;
	}
	
	// Initialize `DIAGONAL_SPAN`
	for (int square = 0; square < 64; square++) {
		const Bitboard S = square_to_bitboard(square);
		for (Bitboard O = 0; O < 256; O++) {
			
			// Figure out the index of `square` in its diagonal
			const int index = popcount(DIAGONALS[square] & (S - 1));
			
			const Bitboard SPAN = HORIZONTAL_SPAN[index][O];
			
			// Rotate `SPAN` back to the diagonal
			Bitboard rSPAN = 0;
			Bitboard DIAGONAL = DIAGONALS[square];
			int i = 0;
			while (DIAGONAL) {
				const int diagonal_square = pop_lsb(DIAGONAL);
				if (SPAN & square_to_bitboard(i))
					rSPAN |= square_to_bitboard(diagonal_square);
				i++;
			}
			
			DIAGONAL_SPAN[square][O] = rSPAN;
		}
	}
	
	// Initialize `ANTI_DIAGONAL_MULTIPLICAND`
	for (int square = 0; square < 64; square++) {
		Bitboard M = 0;
		Bitboard ANTI_DIAGONAL = ANTI_DIAGONALS[square];
		int i = 0;
		while (ANTI_DIAGONAL) {
			/*
			 
			 We use `pop_msb` here because this multiplicand should reverse the rank. For example,
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				4 . . . . . . .
				. 3 . . . . . .
				. . 2 . . . . .
				. . . 1 . . . .
			 should map to
				4 3 2 1 . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
			 when multiplied by `M`. It would make more sense to map to
				1 2 3 4 . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
				. . . . . . . .
			 but it is impossible to find a multiplicand that does this without causing collisions.
			 
			 */
			
			const Bitboard ORIGIN = square_to_bitboard(pop_msb(ANTI_DIAGONAL));
			const Bitboard DESTINATION = square_to_bitboard(A8 + i);
			M |= DESTINATION / ORIGIN;
			i++;
		}
		ANTI_DIAGONAL_MULTIPLICAND[square] = M;
	}
	
	// Initialize `ANTI_DIAGONAL_SPAN`
	for (int square = 0; square < 64; square++) {
		const Bitboard S = square_to_bitboard(square);
		for (Bitboard O = 0; O < 256; O++) {
			
			// Figure out the index of `square` in its anti-diagonal
			const int index = popcount(ANTI_DIAGONALS[square] & (-S ^ S));
			
			const Bitboard SPAN = HORIZONTAL_SPAN[index][O];
			
			// Rotate `SPAN` back to the anti-diagonal
			Bitboard rSPAN = 0;
			Bitboard ANTI_DIAGONAL = ANTI_DIAGONALS[square];
			int i = 0;
			while (ANTI_DIAGONAL) {
				// We use `pop_msb` here to reverse `rSPAN` and counter the rank reversal that happens when multiplying by `ANTI_DIAGONAL_MULTIPLICAND[square]`.
				const int anti_diagonal_square = pop_msb(ANTI_DIAGONAL);
				if (SPAN & square_to_bitboard(i))
					rSPAN |= square_to_bitboard(anti_diagonal_square);
				i++;
			}
			
			ANTI_DIAGONAL_SPAN[square][O] = rSPAN;
		}
	}
	
	// Initialize `PIECE_SCORES`
	for (int endgame = false; endgame <= true; endgame++) {
		for (int player = WHITE; player <= BLACK; player++) {
			for (Piece piece = EMPTY; piece <= KING; piece++) {
				for (int square = 0; square < 64; square++) {
					
					int score = 0;
					switch (piece) {
						case EMPTY:
							score = EMPTY_SCORES[square];
							break;
						case PAWN:
							score = PAWN_SCORES[square];
							break;
						case KNIGHT:
							score = KNIGHT_SCORES[square];
							break;
						case BISHOP:
							score = BISHOP_SCORES[square];
							break;
						case ROOK:
							score = ROOK_SCORES[square];
							break;
						case QUEEN:
							score = QUEEN_SCORES[square];
							break;
						case KING:
							score = endgame ? KING_SCORES_ENDGAME[square] : KING_SCORES_MIDDLEGAME[square];
							break;
					}
					
					score += endgame ? MATERIAL_SCORES_ENDGAME[piece] : MATERIAL_SCORES_MIDDLEGAME[piece];
					
					int y = square / 8, x = square % 8;
					// Flip this square vertically if the player is white
					if (player == WHITE)
						y = 7 - y;
					int new_square = 8 * y + x;
					PIECE_SCORES[endgame][player][piece][new_square] = score;
				}
			}
		}
	}
}

} // namespace Magic

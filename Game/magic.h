//
//  magic.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#pragma once
#ifndef magic_h
#define magic_h

#include "fruit.h"
#include "bitboard.h"

namespace Magic
{

void init();

// MARK: - Move Generation

constexpr int PAWN_PUSH_AMOUNT[2] = {8, -8};
constexpr int PAWN_RIGHT_CAPTURE_AMOUNT[2] = {9, -7};
constexpr int PAWN_LEFT_CAPTURE_AMOUNT[2] = {7, -9};
constexpr Bitboard PROMOTION_RANK[2] = {0xffULL << A8, 0xffULL};
constexpr Bitboard MIDDLE_RANK[2] = {0xffULL << A4, 0xffULL << A5};

// TODO: CONSIDER WHETHER TO TURN THESE extern VARIABLES INTO inline

/// Contains D4, E4, D5, E5.
constexpr Bitboard CENTER_FOUR_SQUARES = (3ULL << D4) | (3ULL << D5);
/// Contains the rectangle of squares that lie around the center four squares.
extern Bitboard RING_OF_RADIUS_2;
/// Contains the rectangle of squares that lie around `RING_OF_RADIUS_2`.
extern Bitboard RING_OF_RADIUS_3;
/// Contains all squares that lie on the edge of the board.
extern Bitboard EDGE_SQUARES;

extern int START_OF_RANK[64];
extern Bitboard HORIZONTAL_SPAN[64][256];

extern Bitboard VERTICAL_MULTIPLICAND[64];
extern Bitboard VERTICAL_SPAN[64][256];

extern Bitboard DIAGONAL_MULTIPLICAND[64];
extern Bitboard DIAGONAL_SPAN[64][256];

extern Bitboard ANTI_DIAGONAL_MULTIPLICAND[64];
extern Bitboard ANTI_DIAGONAL_SPAN[64][256];


// MARK: - Material & Positional Scores

/// General estimate of a piece's material score.
constexpr int BASE_MATERIAL_SCORE[PIECE_COUNT] =
{
	0, 100, 300, 300, 500, 900, 0
};

constexpr int ENDGAME_PROGRESS[PIECE_COUNT] =
{
	0, 0, 1, 1, 2, 4, 0
};

constexpr int MATERIAL_SCORES_MIDDLEGAME[PIECE_COUNT] =
{
	0, 90, 290, 300, 500, 900, 0
};
constexpr int MATERIAL_SCORES_ENDGAME[PIECE_COUNT] =
{
	0, 120, 250, 310, 540, 940, 0
};

constexpr int EMPTY_SCORES[64] =
{
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
constexpr int PAWN_SCORES[64] =
{
	  0,   0,   0,   0,   0,   0,   0,   0,
	 40,  40,  40,  40,  40,  40,  40,  40,
	 30,  30,  30,  35,  35,  30,  30,  30,
	 15,  20,  20,  30,  30,  20,  20,  15,
	  5,   5,  10,  30,  30,  10,   5,   5,
	  0,   5,  -5, -10, -10,  -5,   5,   0,
	  0,   0,   0, -10, -10,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
constexpr int KNIGHT_SCORES[64] =
{
	-30, -25, -20, -20, -20, -20, -25, -30,
	-25,   0,   0,   0,   0,   0,   0, -25,
	-20,   0,   0,   5,   5,   0,   0, -20,
	-20,   0,   5,  15,  15,   5,   0, -20,
	-20,   5,   5,  15,  15,   5,   5, -20,
	-20,   0,   5,   0,   0,   5,   0, -20,
	-25,   0,   0,   0,   0,   0,   0, -25,
	-30, -10, -20, -20, -20, -20, -10, -30,
};
constexpr int BISHOP_SCORES[64] =
{
	 -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,   5,   0,   5,   5,   0,   5,  -5,
	 -5,   0,  10,   5,   5,  10,   0,  -5,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,  20,   0,   0,   0,   0,  20,  -5,
	 -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
};
constexpr int ROOK_SCORES[64] =
{
	  0,   0,   0,   0,   0,   0,   0,   0,
	 10,  10,  10,  15,  15,  10,  10,  10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	  0,   0,   0,  15,  15,   0,   0,   0,
};
constexpr int QUEEN_SCORES[64] =
{
	-10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   5,   5,   5,   5,   0,  -5,
	 -5,   0,   5,   5,   5,   5,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   0,  10,  10,   0,   0,  -5,
	-10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
};
constexpr int KING_SCORES_MIDDLEGAME[64] =
{
	-30, -30, -35, -35, -35, -35, -30, -30,
	-25, -25, -30, -30, -30, -30, -25, -25,
	-20, -20, -25, -25, -25, -25, -20, -20,
	-15, -15, -20, -20, -20, -20, -15, -15,
	-10, -10, -15, -15, -15, -15, -10, -10,
	 -5,  -5, -10, -10, -10, -10,  -5,  -5,
	  0,   0,   0,   0,   0,   0,   0,   0,
	 10,  50,  70,   0,   0,   5,  70,  40,
};
constexpr int KING_SCORES_ENDGAME[64] =
{
	-20, -10,  -5,  -5,  -5,  -5, -10, -20,
	-10,   0,   0,   0,   0,   0,   0, -10,
	  0,  25,  30,  30,  30,  30,  25,   0,
	  0,  20,  30,  40,  40,  30,  20,   0,
	  5,   5,  30,  40,  40,  30,   5,   5,
	-10,   5,  20,  30,  30,  20,   5, -10,
	-30, -15,   0,   0,  0,   0, -15, -30,
	-40, -30, -15,  -5,  -5, -15, -30, -40,
};

/// Usage: `PIECE_SCORES[endgame][player][piece][square]`.
extern int PIECE_SCORES[2][2][PIECE_COUNT][64];

} // namespace Magic

#endif /* magic_h */

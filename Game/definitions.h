//
//  definitions.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#pragma once
#ifndef definitions_h
#define definitions_h

#include "fruit.h"
#include <cstdint>

typedef uint64_t Bitboard;


enum Square: short
{
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
};


typedef bool Color;
constexpr int PLAYER_COUNT = 2;
constexpr Color WHITE = 0, BLACK = 1;


typedef short Piece;
constexpr int PIECE_COUNT = 7;
constexpr Piece EMPTY = 0, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;


typedef short CastlingRights;
constexpr CastlingRights CASTLE_KINGSIDE[2] = {1 << WHITE, 1 << BLACK};
constexpr CastlingRights CASTLE_QUEENSIDE[2] = {4 << WHITE, 4 << BLACK};


typedef uint64_t Move;
constexpr Move NULL_MOVE = 0;

/*
 
 enpassant_square(6) captured_piece_color(1) captured_piece(3) promotion(3) piece(3) to(6) from(6)
 
 Total bits: 28 / 64
 
 */

constexpr int FROM_OFFSET = 0;
constexpr int TO_OFFSET = 6;
constexpr int PIECE_OFFSET = 12;
constexpr int PROMOTION_OFFSET = 15;
constexpr int CAPTURED_PIECE_OFFSET = 18;
constexpr int CAPTURED_PIECE_COLOR_OFFSET = 21;
constexpr int EN_PASSANT_SQUARE_OFFSET = 22;

inline Move create_promotion_capture_move(int from, int to, Piece piece, Piece promotion, Piece captured_piece, Color captured_piece_color, int enpassant_square = 0)
{
	Move move = 0;
	move |= (Move)from << FROM_OFFSET;
	move |= (Move)to << TO_OFFSET;
	move |= (Move)piece << PIECE_OFFSET;
	move |= (Move)promotion << PROMOTION_OFFSET;
	move |= (Move)captured_piece << CAPTURED_PIECE_OFFSET;
	move |= (Move)captured_piece_color << CAPTURED_PIECE_COLOR_OFFSET;
	move |= (Move)enpassant_square << EN_PASSANT_SQUARE_OFFSET;
	return move;
}
inline Move create_capture_move(int from, int to, Piece piece, Piece captured_piece, Color captured_piece_color, int enpassant_square = 0)
{
	return create_promotion_capture_move(from, to, piece, piece, captured_piece, captured_piece_color, enpassant_square);
}
inline Move create_promotion_move(int from, int to, Piece piece, Piece promotion, int enpassant_square = 0)
{
	return create_promotion_capture_move(from, to, piece, promotion, EMPTY, WHITE, enpassant_square);
}
inline Move create_move(int from, int to, Piece piece, int enpassant_square = 0)
{
	return create_promotion_capture_move(from, to, piece, piece, EMPTY, WHITE, enpassant_square);
}

inline int move_from(Move move)
{
	return (int)(move >> FROM_OFFSET) & 63;
}
inline int move_to(Move move)
{
	return (int)(move >> TO_OFFSET) & 63;
}
inline Piece move_piece(Move move)
{
	return (Piece)(move >> PIECE_OFFSET) & 7;
}
inline Piece move_promotion(Move move)
{
	return (Piece)(move >> PROMOTION_OFFSET) & 7;
}
inline Piece move_captured_piece(Move move)
{
	return (Piece)(move >> CAPTURED_PIECE_OFFSET) & 7;
}
inline Color move_captured_piece_color(Move move)
{
	// This appears to be slightly faster than `(Color)(move >> CAPTURED_PIECE_COLOR_OFFSET)`
	return (Color)(move & (1ULL << CAPTURED_PIECE_COLOR_OFFSET));
}
inline int move_enpassant_square(Move move)
{
	return (int)(move >> EN_PASSANT_SQUARE_OFFSET) & 63;
}

/// Returns whether `move` is considered to be irreversible for the purpose of the 50-move rule -- that is, if `move` is a capture or a pawn move. Note that castling moves and moves that remove a player's castling rights are technically "irreversible" but are treated as reversible for the 50-move rule.
inline bool is_irreversible(Move move)
{
	return move_captured_piece(move) || move_piece(move) == PAWN;
}


typedef int Direction;
constexpr Direction NORTH = 0, NORTH_EAST = 1, EAST = 2, SOUTH_EAST = 3, SOUTH = 4, SOUTH_WEST = 5, WEST = 6, NORTH_WEST = 7;


typedef uint64_t HashKey;

#endif /* definitions_h */

//
//  notation.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#pragma once
#ifndef notation_h
#define notation_h

#include "fruit.h"
#include "definitions.h"
#include "bitboard.h"
#include "game_def.h"

namespace Notation
{

int parse_square(const std::string &algebraic);
std::string square_to_string(int square);
Piece char_to_piece(char ch);
char piece_to_char(Piece piece, Color player);
char piece_to_char(Piece piece);

std::string move_to_string(Move move);

template<Variant V>
Move parse_move(const std::string &algebraic, const Game<V> &game)
{
	if (!(algebraic.length() == 4 || algebraic.length() == 5))
		return 0;
	
	int from = parse_square(fruit::string(algebraic[0]) + fruit::string(algebraic[1]));
	int to = parse_square(fruit::string(algebraic[2]) + fruit::string(algebraic[3]));
	
	Piece piece = game.list[from];
	Piece captured_piece = game.list[to];
	
	// Promotion
	Piece promotion = piece;
	if (algebraic.length() == 5) {
		promotion = Notation::char_to_piece(algebraic[4]);
		if (promotion == EMPTY)
			return 0;
	}
	
	// En passant
	int enpassant_square = 0;
	if (piece == PAWN) {
		if (to == from + 16)
			enpassant_square = to - 8;
		else if (to == from - 16)
			enpassant_square = to + 8;
	}
	return create_promotion_capture_move(from, to, piece, promotion, captured_piece, !game.active_player, enpassant_square);
}

Bitboard parse_bitboard(const std::string &s);
void display(Bitboard B, const std::string &label = "");

std::string variant_to_string(const Variant variant);
Variant parse_variant(const std::string &name);


// MARK: - FEN

const std::string default_setup_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string kiwi_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
const std::string mango_fen = "rn1qk2r/pbp2pp1/1p1bpn1p/1N1p4/3P4/P3PN2/1PPBQPPP/3RK2R w Kkq -";
const std::string grape_fen = "3Q4/8/3p4/3N3P/7P/1k6/8/K1R5 w - -";

const std::map<std::string, std::string> named_positions =
{
	{"start", default_setup_fen},
	{"kiwi", kiwi_fen},
	{"mango", mango_fen},
	{"grape", grape_fen},
};


// MARK: - Universal Notation

std::string color_to_universal(Color player);
std::string piece_to_universal(Piece piece);
std::string square_to_universal(int square);
std::string variant_to_universal(Variant variant);

Color universal_to_color(const std::string &universal);
Piece universal_to_piece(const std::string &universal);
int universal_to_square(const std::string &universal);
Variant universal_to_variant(const std::string &universal);

__attribute__((noreturn))
void invalid_universal_string(const std::string &universal_type, const std::string &universal);

} // namespace Notation

#endif /* notation_h */

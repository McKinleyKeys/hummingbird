//
//  notation.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#include "notation.h"

namespace Notation
{

int parse_square(const std::string &algebraic)
{
	int x = algebraic[0] - 'a';
	int y = algebraic[1] - '1';
	return 8 * y + x;
}
std::string square_to_string(int square)
{
	std::string algebraic = "";
	algebraic += 'a' + (square % 8);
	algebraic += '1' + (square / 8);
	return algebraic;
}
Piece char_to_piece(char ch)
{
	if (ch == 'p' || ch == 'P') return PAWN;
	if (ch == 'n' || ch == 'N') return KNIGHT;
	if (ch == 'b' || ch == 'B') return BISHOP;
	if (ch == 'r' || ch == 'R') return ROOK;
	if (ch == 'q' || ch == 'Q') return QUEEN;
	if (ch == 'k' || ch == 'K') return KING;
	return EMPTY;
}
char piece_to_char(Piece piece, Color player)
{
	char chars[2][PIECE_COUNT] = {
		{'.', 'P', 'N', 'B', 'R', 'Q', 'K'},
		{'.', 'p', 'n', 'b', 'r', 'q', 'k'}
	};
	return chars[player][piece];
}
char piece_to_char(Piece piece)
{
	return piece_to_char(piece, WHITE);
}

std::string move_to_string(Move move)
{
	if (move == NULL_MOVE)
		return "null";
	std::string s = "";
	s += square_to_string(move_from(move));
	s += square_to_string(move_to(move));
	if (move_promotion(move) != move_piece(move))
		s += piece_to_char(move_promotion(move), BLACK);
	return s;
}

Bitboard parse_bitboard(const std::string &s)
{
	Bitboard B = 0;
	int y = 7, x = 0;
	for (char ch : s) {
		
		// Ignore whitespace
		if (std::isspace(ch))
			continue;
		
		if (y < 0)
			fruit::fatal_error("Bitboard contains more than 64 squares");
		
		const Bitboard S = square_to_bitboard(8 * y + x);
		switch (ch) {
			case '1': case 'x': case 'X':
				B |= S;
				break;
			case '0': case '.': case '_':
				break;
			default:
				fruit::fatal_error("Invalid character in bitboard: " + fruit::debug_description(ch));
		}
		
		x++;
		if (x == 8) {
			x = 0;
			y--;
		}
	}
	return B;
}
void display(Bitboard B, const std::string &label)
{
	std::string result = "";
	if (label.size()) {
		result += label + ":\n";
	}
	for (int y = 8; y --> 0;) {
		for (int x = 0; x < 8; x++) {
			int square = 8 * y + x;
			result += (B & square_to_bitboard(square)) ? '1' : '.';
			if (x < 7)
				result += ' ';
		}
		result += '\n';
	}
	cout << result << endl;
}


std::string variant_to_string(const Variant variant)
{
	switch (variant) {
		case CLASSIC: return "Classic";
//		case ENERGY_CELLS: return "Energy cells";
		case EXPLODING_KNIGHTS: return "Exploding knights";
		case COMPULSION: return "Compulsion";
		case COMPULSION_AND_BACKSTABBING: return "Compulsion and backstabbing";
//		case PETRIFICATION: return "Petrification";
		case FORCED_CHECK: return "Forced check";
		case FORCED_CHECK_AND_BACKSTABBING: return "Forced check and backstabbing";
		case LOSER: return "Loser's";
		case KING_OF_THE_HILL: return "King of the hill";
		case KING_OF_THE_HILL_AND_COMPULSION: return "King of the hill and compulsion";
//		case THREE_CHECK: return "Three-check";
//		case FOG_OF_WAR: return "Fog of war";
		case UNRECOGNIZED_VARIANT: return "Unrecognized variant";
	}
}
Variant parse_variant(const std::string &string)
{
	auto fix_name = [](const std::string name) -> std::string {
		std::string name_fixed = name;
		// Remove whitespace and punctuation
		name_fixed = fruit::removing_characters_where(name_fixed, [](char ch) {
			return std::isspace(ch) || std::ispunct(ch);
		});
		// Make lowercase
		name_fixed = fruit::to_lowercase(name_fixed);
		return name_fixed;
	};
	
	const std::string string_fixed = fix_name(string);
	
	for (int variant = 0; variant < VARIANT_COUNT; variant++) {
		const std::string variant_name = variant_to_string((Variant)variant);
		const std::string variant_name_fixed = fix_name(variant_name);
		if (variant_name_fixed == string_fixed)
			return (Variant)variant;
	}
	return UNRECOGNIZED_VARIANT;
}


// MARK: - Universal Notation

std::string color_to_universal(Color player)
{
	return player == WHITE ? "w" : "b";
}
std::string piece_to_universal(Piece piece)
{
	return fruit::string(piece_to_char(piece, BLACK));
}
std::string square_to_universal(int square)
{
	return square_to_string(square);
}
std::string variant_to_universal(Variant variant)
{
	switch (variant) {
		case CLASSIC: return "classic";
//		case ENERGY_CELLS: return "energyCells";
		case EXPLODING_KNIGHTS: return "explodingKnights";
		case COMPULSION: return "compulsion";
		case COMPULSION_AND_BACKSTABBING: return "compulsionAndBackstabbing";
//		case PETRIFICATION: return "petrification";
		case FORCED_CHECK: return "forcedCheck";
		case FORCED_CHECK_AND_BACKSTABBING: return "forcedCheckAndBackstabbing";
		case LOSER: return "loser";
		case KING_OF_THE_HILL: return "kingOfTheHill";
		case KING_OF_THE_HILL_AND_COMPULSION: return "kingOfTheHillAndCompulsion";
//		case THREE_CHECK: return "threeCheck";
//		case FOG_OF_WAR: return "fogOfWar";
		default:
			cout << "(Warning) attempted to convert invalid variant " << fruit::debug_description(variant) << " to universal notation" << endl;
			return "";
	}
}


Color universal_to_color(const std::string &universal)
{
	if (universal == "w") return WHITE;
	if (universal == "b") return BLACK;
	invalid_universal_string("color", universal);
}
Piece universal_to_piece(const std::string &universal)
{
	if (universal.size() == 1) {
		Piece piece = char_to_piece(universal[0]);
		if (piece)
			return piece;
	}
	invalid_universal_string("piece", universal);
}
int universal_to_square(const std::string &universal)
{
	return parse_square(universal);
}
Variant universal_to_variant(const std::string &universal)
{
	for (int variant = 0; variant < VARIANT_COUNT; variant++)
		if (variant_to_universal((Variant)variant) == universal)
			return (Variant)variant;
	invalid_universal_string("variant", universal);
}

__attribute__((noreturn))
void invalid_universal_string(const std::string &universal_type, const std::string &universal)
{
	fruit::fatal_error("Invalid universal " + universal_type + " string: " + fruit::debug_description(universal));
}

} // namespace Notation

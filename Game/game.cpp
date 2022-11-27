//
//  game.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#include "game.h"

template<Variant V>
Game<V>::Game()
{
	quasilegal_moves.reserve(40);
	
	const int foreseeable_future = 256;
	move_history.reserve(foreseeable_future);
	castling_rights_history.reserve(foreseeable_future);
	EN_PASSANT_HISTORY.reserve(foreseeable_future);
	hash_history.reserve(foreseeable_future);
	reversible_move_clock_history.reserve(foreseeable_future);
	if constexpr (Variants::has_destructive_moves(V)) {
		// These vectors will only be appended to when a destructive move is played
		PIECES_HISTORY.reserve(foreseeable_future);
		PLAYERS_HISTORY.reserve(foreseeable_future);
		list_history.reserve(foreseeable_future);
	}
}

std::unique_ptr<AbstractGame> AbstractGame::instantiate(Variant variant)
{
	switch (variant) {
		#define CASE_STATEMENT_FOR_VARIANT(V) \
			case V: \
				return std::make_unique<Game<V>>(); \
				break;
		FOR_EACH_VARIANT(CASE_STATEMENT_FOR_VARIANT)
		#undef CASE_STATEMENT_FOR_VARIANT
		default:
			return nullptr;
	}
}

template<Variant V>
std::unique_ptr<AbstractGame> Game<V>::clone() const
{
	return std::make_unique<Game<V>>(*this);
}


template<Variant V>
void Game<V>::clear()
{
	active_player = WHITE;
	
	// Board
	for (Piece piece = EMPTY; piece <= KING; piece++) {
		PIECES[piece] = 0;
	}
	PLAYERS[WHITE] = 0;
	PLAYERS[BLACK] = 0;
	OCCUPIED = 0;
	for (int square = 0; square < 64; square++) {
		list[square] = EMPTY;
	}
	
	// Castling
	castling_rights = 0;
	
	// En passant
	EN_PASSANT = 0;
	
	// History
	move_history.clear();
	castling_rights_history.clear();
	EN_PASSANT_HISTORY.clear();
	reversible_move_clock_history.clear();
	
//	repetition_count.clear();
//	repetition_count_current = 0;
	reversible_move_clock = 0;
	
	hash = 0;
}

template<Variant V>
void Game<V>::set(int square, Piece piece, Color player)
{
	Bitboard S = square_to_bitboard(square);
	PIECES[piece] |= S;
	PLAYERS[player] |= S;
	OCCUPIED |= S;
	list[square] = piece;
}

template<Variant V>
void Game<V>::set(const std::string &algebraic, Piece piece, Color player)
{
	set(Notation::parse_square(algebraic), piece, player);
}

template<Variant V>
void Game<V>::default_setup()
{
	setup_fen(Notation::default_setup_fen);
}

template<Variant V>
void Game<V>::setup_fen(const std::string &fen)
{
	{
		if (fen == "") return default_setup();
		const auto result = Notation::named_positions.find(fen);
		if (result != Notation::named_positions.end())
			return setup_fen(result->second);
	}
	
	clear();
	
	// Board
	int y = 7, x = 0, i = 0;
	for (; i < fen.length(); i++) {
		
		if (fen[i] == ' ') break;
		if (fen[i] == '/') continue;
		
		if (fen[i] >= '1' && fen[i] <= '9') {
			x += fen[i] - '0';
		}
		else {
			set(8 * y + x, Notation::char_to_piece(fen[i]), fen[i] >= 'a');
			x++;
		}
		
		if (x >= 8) {
			y--;
			x = 0;
		}
	}
	i++;
	
	// Active player
	active_player = fen[i] == 'b';
	i++;
	i++;
	
	// Castling
	if (fen[i] != '-') {
		for (; i < fen.length(); i++) {
			if (fen[i] == ' ') break;
			switch (fen[i]) {
				case 'k':
					castling_rights |= CASTLE_KINGSIDE[BLACK];
					break;
				case 'q':
					castling_rights |= CASTLE_QUEENSIDE[BLACK];
					break;
				case 'K':
					castling_rights |= CASTLE_KINGSIDE[WHITE];
					break;
				case 'Q':
					castling_rights |= CASTLE_QUEENSIDE[WHITE];
					break;
			}
		}
	}
	else i++;
	i++;
	
	// En passant
	if (fen[i] != '-') {
		std::string enpassant_string = "";
		enpassant_string += fen[i];
		enpassant_string += fen[i + 1];
		EN_PASSANT = square_to_bitboard(Notation::parse_square(enpassant_string));
	}
	
	sync_hash();
	
	// Repetition count
//	repetition_count_current = ++repetition_count[hash];
}

template<Variant V>
void Game<V>::setup_visual(const std::string &visual)
{
	{
		if (visual == "") return default_setup();
		const auto result = Notation::named_positions.find(visual);
		if (result != Notation::named_positions.end())
			return setup_fen(result->second);
	}
	
	clear();
	
	const std::vector<std::string> lines = fruit::split(visual, '\n');
	for (int y = 8; y --> 0;) {
		const std::vector<std::string> rank = fruit::split(fruit::trimming_whitespace(lines[7 - y]), ' ');
		for (int x = 0; x < 8; x++) {
			
			const std::string &substring = rank[x];
			if (substring == ".")
				continue;
			const int square = 8 * y + x;
			Piece piece;
			if (substring.size() == 1 && (piece = Notation::char_to_piece(substring[0]))) {
				Color player = std::isupper(substring[0]) ? WHITE : BLACK;
				set(square, piece, player);
			}
			else
				fruit::fatal_error("Visual board representation contains invalid character: " + fruit::debug_description(substring));
		}
	}
	const std::vector<std::string> final_line = fruit::split(fruit::trimming_whitespace(lines[8]), ' ');
	
	// Active player
	const std::string &player_string = final_line[0];
	if (!(player_string == "w" || player_string == "b"))
		fruit::fatal_error("Visual board representation contains invalid active player: " + fruit::debug_description(player_string));
	active_player = player_string == "w" ? WHITE : BLACK;
	
	// Castling
	const std::string &castle_string = final_line[1];
	if (castle_string.size() <= 0 || castle_string.size() > 4)
		fruit::fatal_error("Visual board representation contains invalid castling information: " + fruit::debug_description(castle_string));
	if (castle_string != "-") {
		for (char ch : castle_string) {
			switch (ch) {
				case 'k': castling_rights |= CASTLE_KINGSIDE[BLACK]; break;
				case 'q': castling_rights |= CASTLE_QUEENSIDE[BLACK]; break;
				case 'K': castling_rights |= CASTLE_KINGSIDE[WHITE]; break;
				case 'Q': castling_rights |= CASTLE_QUEENSIDE[WHITE]; break;
				case '-':
					fruit::fatal_error("Visual board representation contains dash in castling information: " + fruit::debug_description(castle_string));
				default:
					fruit::fatal_error("Visual board representation contains unrecognized character in castling information: " + fruit::debug_description(castle_string));
			}
		}
	}
	
	// En passant
	const std::string &enpassant_string = final_line[2];
	if (enpassant_string != "-" && enpassant_string != "*") {
		if (enpassant_string.size() != 2)
			fruit::fatal_error("Visual board representation contains invalid en passant information: " + fruit::debug_description(enpassant_string));
		int x = enpassant_string[0] - 'a';
		int y = enpassant_string[1] - '1';
		if (x < 0 || x >= 8 || y < 0 || y >= 8)
			fruit::fatal_error("Visual board representation contains invalid en passant square: " + fruit::debug_description(enpassant_string));
		EN_PASSANT = square_to_bitboard(y * 8 + x);
	}
	
	sync_hash();
	
	// Repetition count
//	repetition_count_current = ++repetition_count[hash];
}

template<Variant V>
void Game<V>::sync_hash()
{
	hash = 0;
	
	// Board
	for (int square = 0; square < 64; square++) {
		Bitboard S = square_to_bitboard(square);
		for (int player = WHITE; player <= BLACK; player++) {
			for (Piece piece = EMPTY; piece <= KING; piece++) {
				if (PIECES[piece] & PLAYERS[player] & S)
					hash ^= Zobrist::keys[square][player][piece];
			}
		}
	}
	
	// Active player
	if (active_player) {
		hash ^= Zobrist::active_player_key;
	}
	
	// Castling
	for (int player = WHITE; player <= BLACK; player++) {
		if (castling_rights & CASTLE_KINGSIDE[player])
			hash ^= Zobrist::kingside_castling_keys[player];
		if (castling_rights & CASTLE_QUEENSIDE[player])
			hash ^= Zobrist::queenside_castling_keys[player];
	}
	
	// En passant
	if (EN_PASSANT) {
		int file = lsb(EN_PASSANT) % 8;
		hash ^= Zobrist::enpassant_keys[file];
	}
}


template<Variant V>
std::string Game<V>::fen() const
{
	std::string s = "";
	// Board
	for (int y = 8; y --> 0;) {
		
		int empty_count = 0;
		for (int x = 0; x < 8; x++) {
			
			int square = 8 * y + x;
			char ch = Notation::piece_to_char(list[square], PLAYERS[BLACK] & square_to_bitboard(square));
			if (ch != '.') {
				// Flush the empty squares
				if (empty_count) {
					s += '0' + empty_count;
					empty_count = 0;
				}
				s += ch;
			}
			else empty_count++;
		}
		
		// Flush the empty squares
		if (empty_count) {
			s += '0' + empty_count;
			empty_count = 0;
		}
		if (y) s += '/';
	}
	s += ' ';
	
	// Active player
	s += (active_player == WHITE) ? 'w' : 'b';
	s += ' ';
	
	// Castling
	std::string castling_string = "";
	for (int player = WHITE; player <= BLACK; player++) {
		if (castling_rights & CASTLE_KINGSIDE[player])
			castling_string += (player == WHITE) ? 'K' : 'k';
		if (castling_rights & CASTLE_QUEENSIDE[player])
			castling_string += (player == WHITE) ? 'Q' : 'q';
	}
	s += (castling_string == "") ? "-" : castling_string;
	s += ' ';
	
	// En passant
	if (EN_PASSANT) {
		int square = lsb(EN_PASSANT);
		s += Notation::square_to_string(square);
	}
	else s += '-';
	s += ' ';
	
	// Fifty-move clock
	s += '0';
	s += ' ';
	
	// Turn number
	s += '1';
	
	return s;
}

template<Variant V>
std::string Game<V>::debug_description() const
{
	std::string string = "";
	for (int y = 8; y --> 0;) {
		for (int x = 0; x < 8; x++) {
			const int square = 8 * y + x;
			string += Notation::piece_to_char(list[square], PLAYERS[BLACK] & square_to_bitboard(square));
			if (x < 7)
				string += ' ';
		}
		if (y > 0)
			string += '\n';
	}
	return string;
}

template<Variant V>
void Game<V>::display(const std::string &label) const
{
	if (label.size())
		cout << label << ":" << endl;
	cout << debug_description() << endl;
	cout << endl;
}

/// In order for the visual entries in opening books to be matched with games correctly, this method must always return a string that has no lines that start or end with whitespace.
template<Variant V>
std::string Game<V>::visual() const
{
	std::string string = debug_description();
	string += '\n';
	
	// Active player
	string += (active_player == WHITE) ? 'w' : 'b';
	string += ' ';
	
	// Castling
	std::string castling_string = "";
	for (int player = WHITE; player <= BLACK; player++) {
		if (castling_rights & CASTLE_KINGSIDE[player])
			castling_string += (player == WHITE) ? 'K' : 'k';
		if (castling_rights & CASTLE_QUEENSIDE[player])
			castling_string += (player == WHITE) ? 'Q' : 'q';
	}
	if (castling_string == "")
		castling_string = "-";
	string += castling_string;
	string += ' ';
	
	// En passant
	if (EN_PASSANT)
		string += Notation::square_to_string(lsb(EN_PASSANT));
	else
		string += '-';
	
	return string;
}


// MARK: - Notation

template<Variant V>
std::string Game<V>::universal_notation(Move move) const
{
	std::string universal = "";
	universal += Notation::square_to_string(move_from(move));
	universal += Notation::square_to_string(move_to(move));
	Piece promotion = move_promotion(move);
	if (promotion != move_piece(move)) {
		universal += Notation::color_to_universal(active_player);
		universal += Notation::piece_to_universal(promotion);
	}
	return universal;
}

template<Variant V>
Move Game<V>::try_parse_universal(const std::string &universal) const
{
	if (!(universal.size() == 4 || universal.size() == 6))
		return NULL_MOVE;
	int from = Notation::parse_square(universal.substr(0, 2));
	int to = Notation::parse_square(universal.substr(2, 2));
	Piece piece = list[from];
	Piece captured_piece = list[to];
	Color captured_piece_color = color_at_square(square_to_bitboard(to));
	
	Piece promotion_piece = piece;
	if (universal.size() == 6)
		promotion_piece = Notation::universal_to_piece(universal.substr(5, 1));
	
	// TODO: Check for the color of the promotion piece
	// `universal = f7f8wq`
	
	int enpassant_square = 0;
	if (piece == PAWN) {
		if (to == from + 16)
			enpassant_square = to - 8;
		else if (to == from - 16)
			enpassant_square = to + 8;
	}
	
	return create_promotion_capture_move(from, to, piece, promotion_piece, captured_piece, captured_piece_color, enpassant_square);
}
template<Variant V>
Move Game<V>::parse_universal(const std::string &universal) const
{
	Move move = try_parse_universal(universal);
	if (move == NULL_MOVE)
		Notation::invalid_universal_string("move", universal);
	return move;
}

template<Variant V>
Move Game<V>::parse_algebraic(const std::string &_algebraic) const
{
	std::string algebraic = _algebraic;
	
	if (algebraic == "null")
		return NULL_MOVE;
	
	// Castling
	if (algebraic == "O-O") {
		if (active_player == WHITE)
			return create_move(E1, G1, KING);
		else
			return create_move(E8, G8, KING);
	}
	if (algebraic == "O-O-O") {
		if (active_player == WHITE)
			return create_move(E1, C1, KING);
		else
			return create_move(E8, C8, KING);
	}
	
	// Get piece type
	Piece piece = PAWN;
	if (algebraic != "" && std::isupper(algebraic.front())) {
		char letter = algebraic.front();
		algebraic.erase(algebraic.begin());
		piece = Notation::char_to_piece(letter);
	}
	
	// Check or checkmate
	while (algebraic.back() == '#' || algebraic.back() == '+')
		algebraic.pop_back();
	
	// Promotion
	Piece promotion = piece;
	if (algebraic.size() >= 2 && algebraic[algebraic.size() - 2] == '=') {
		promotion = Notation::char_to_piece(algebraic.back());
		algebraic.pop_back();
		algebraic.pop_back();
	}
	
	// Destination square
	if (algebraic.size() < 2)
		return NULL_MOVE;
	std::string destination_string = algebraic.substr(algebraic.size() - 2);
	int destination = Notation::parse_square(destination_string);
	algebraic.pop_back();
	algebraic.pop_back();
	
	// Capturing
	if (algebraic.back() == 'x')
		algebraic.pop_back();
	
	// Get any additional ambiguity specifiers
	int x_ambiguity = -1;
	int y_ambiguity = -1;
	if (algebraic != "" && algebraic.front() >= 'a' && algebraic.front() <= 'h') {
		x_ambiguity = algebraic.front() - 'a';
		algebraic.erase(algebraic.begin());
	}
	if (algebraic != "" && algebraic.front() >= '1' && algebraic.front() <= '8') {
		y_ambiguity = algebraic.front() - '1';
		algebraic.erase(algebraic.begin());
	}
	
	if (algebraic != "")
		return NULL_MOVE;
	
	// Search through all possible moves for one that fits the parameters
	generate_quasilegal_moves();
	for (Move move : quasilegal_moves) {
		if (move_piece(move) == piece && move_to(move) == destination && move_promotion(move) == promotion) {
			// Check ambiguity
			int origin = move_from(move);
			int origin_y = origin / 8, origin_x = origin % 8;
			if ((x_ambiguity == -1 || origin_x == x_ambiguity) && (y_ambiguity == -1 || origin_y == y_ambiguity)) {
				// We found a valid move
				return move;
			}
		}
	}
	return NULL_MOVE;
}


template<Variant V>
void Game<V>::sanity_check() const
{
	// {square, description}
	std::vector<std::pair<int, std::string>> problems;
	
	for (int square = 0; square < 64; square++) {
		
		Bitboard S = square_to_bitboard(square);
		
		bool white = PLAYERS[WHITE] & S;
		bool black = PLAYERS[BLACK] & S;
		bool pieces[PIECE_COUNT];
		for (Piece piece = 0; piece < PIECE_COUNT; piece++)
			pieces[piece] = PIECES[piece] & S;
		
		bool is_valid = true;
		if (list[square] && !white && !black)
			is_valid = false;
		if (white && black)
			is_valid = false;
		if (list[square] && !pieces[list[square]])
			is_valid = false;
		for (Piece piece = 0; piece < PIECE_COUNT; piece++)
			if (piece != EMPTY && pieces[piece] && list[square] != piece)
				is_valid = false;
		
		if (!is_valid) {
			std::vector<std::string> stats {
				"list=" + fruit::string(Notation::piece_to_char(list[square])),
				"WHITE=" + std::to_string(white),
				"BLACK=" + std::to_string(black),
			};
			for (Piece piece = 0; piece < PIECE_COUNT; piece++)
				if (piece != EMPTY)
					stats.push_back("PIECES[" + fruit::string(Notation::piece_to_char(piece)) + "]=" + std::to_string(pieces[piece]));
			std::string description = fruit::join(stats, ", ");
			problems.emplace_back(square, description);
		}
	}
	
	if (problems.empty())
		cout << "Sanity check found no problems with game" << endl;
	else {
		cout << "Sanity check found the following problems with game:" << endl;
		cout << endl;
		for (auto [square, description] : problems)
			cout << " - " << Notation::square_to_string(square) << ": " << description << endl;
		cout << endl;
	}
}


// MARK: - UI Helpers

template<Variant V>
std::vector<int> Game<V>::captured_pieces(Move move) const
{
	std::vector<int> result;
	
	const int from = move_from(move);
	const int to = move_to(move);
	const Piece piece = move_piece(move);
	const Piece captured_piece = move_captured_piece(move);
	const Bitboard TO = square_to_bitboard(to);
	
	if (captured_piece)
		result.push_back(to);
	
	// En passant
	if (piece == PAWN && (EN_PASSANT & TO)) {
		int captured_pawn;
		if (to > from) {
			// Our pawn pushed up
			captured_pawn = to - 8;
		}
		else {
			// Our pawn pushed down
			captured_pawn = to + 8;
		}
		result.push_back(captured_pawn);
	}
	
	// Explosions
	if constexpr (V == EXPLODING_KNIGHTS) {
		if (piece == KNIGHT && captured_piece) {
			Bitboard BLAST_AREA = (adjacent_squares(to) | TO) & OCCUPIED;
			while (BLAST_AREA) {
				int square = pop_lsb(BLAST_AREA);
				result.push_back(square);
			}
			// Destroy the knight
			result.push_back(from);
		}
	}
	
	return result;
}


// Explicitly instantiate the `Game` class with each variant
//template class Game<CLASSIC>;
//template class Game<ENERGY_CELLS>;
//template class Game<EXPLODING_KNIGHTS>;
//template class Game<COMPULSION>;
//template class Game<COMPULSION_AND_BACKSTABBING>;
//template class Game<PETRIFICATION>;
//template class Game<FORCED_CHECK>;
//template class Game<LOSER>;
//template class Game<KING_OF_THE_HILL>;
//template class Game<KING_OF_THE_HILL_AND_COMPULSION>;
//template class Game<THREE_CHECK>;
//template class Game<FOG_OF_WAR>;

#define INSTANTIATE_GAME(VARIANT) \
	template class Game<VARIANT>;

FOR_EACH_VARIANT(INSTANTIATE_GAME)

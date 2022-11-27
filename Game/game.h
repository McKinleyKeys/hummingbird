//
//  game.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#pragma once
#ifndef game_h
#define game_h

#include "game_def.h"
#include "magic.h"
#include "notation.h"

// MARK: - Move Generation

template<Variant V>
template<Color PLAYER>
inline void Game<V>::generate_quasilegal_moves_for() const
{
	constexpr Color OPPONENT = !PLAYER;
	constexpr Direction FORWARD = (PLAYER == WHITE ? NORTH : SOUTH);
	constexpr Direction FORWARD_EAST = (PLAYER == WHITE ? NORTH_EAST : SOUTH_EAST);
	constexpr Direction FORWARD_WEST = (PLAYER == WHITE ? NORTH_WEST : SOUTH_WEST);
	
	quasilegal_moves.clear();
	
	const Bitboard NON_FRIENDLY = ~PLAYERS[PLAYER];
	const Bitboard ENEMY = PLAYERS[OPPONENT];
	const Bitboard UNOCCUPIED = ~OCCUPIED;
	const Bitboard FRIENDLY_KINGS = PIECES[KING] & PLAYERS[PLAYER];
	
	const Bitboard PAWNS = PIECES[PAWN] & PLAYERS[PLAYER];
	
	// Right capturing
	Bitboard RIGHT = shift<FORWARD_EAST>(PAWNS);
	if constexpr (Variants::has_friendly_fire_enabled(V))
		RIGHT &= (OCCUPIED | EN_PASSANT) & ~FRIENDLY_KINGS;
	else
		RIGHT &= (ENEMY | EN_PASSANT);
	while (RIGHT) {
		int first = pop_lsb(RIGHT);
		Bitboard FIRST = square_to_bitboard(first);
		int origin = first - Magic::PAWN_RIGHT_CAPTURE_AMOUNT[PLAYER];
		Color captured_piece_color = color_at_square(FIRST);
		// Promotion
		if (FIRST & Magic::PROMOTION_RANK[PLAYER]) {
			// Add all possible promotion options
			for (Piece piece = KNIGHT; piece <= QUEEN; piece++) {
				Move move = create_promotion_capture_move(origin, first, PAWN, piece, list[first], captured_piece_color);
				quasilegal_moves.push_back(move);
//				captures.push_back(move);
			}
		}
		else {
			Move move = create_capture_move(origin, first, PAWN, list[first], captured_piece_color);
			quasilegal_moves.push_back(move);
//			captures.push_back(move);
		}
	}
	// Left capturing
	Bitboard LEFT = shift<FORWARD_WEST>(PAWNS);
	if constexpr (Variants::has_friendly_fire_enabled(V))
		LEFT &= (OCCUPIED | EN_PASSANT) & ~FRIENDLY_KINGS;
	else
		LEFT &= (ENEMY | EN_PASSANT);
	while (LEFT) {
		int first = pop_lsb(LEFT);
		Bitboard FIRST = square_to_bitboard(first);
		int origin = first - Magic::PAWN_LEFT_CAPTURE_AMOUNT[PLAYER];
		Color captured_piece_color = color_at_square(FIRST);
		// Promotion
		if (FIRST & Magic::PROMOTION_RANK[PLAYER]) {
			// Add all possible promotion options
			for (Piece piece = KNIGHT; piece <= QUEEN; piece++) {
				Move move = create_promotion_capture_move(origin, first, PAWN, piece, list[first], captured_piece_color);
				quasilegal_moves.push_back(move);
//				captures.push_back(move);
			}
		}
		else {
			Move move = create_capture_move(origin, first, PAWN, list[first], captured_piece_color);
			quasilegal_moves.push_back(move);
//			captures.push_back(move);
		}
	}
	
	// Single push
	const Bitboard PUSHES = shift<FORWARD>(PAWNS) & UNOCCUPIED;
	Bitboard SINGLE = PUSHES;
	while (SINGLE) {
		int first = pop_lsb(SINGLE);
		int origin = first - Magic::PAWN_PUSH_AMOUNT[PLAYER];
		// Promotion
		if (square_to_bitboard(first) & Magic::PROMOTION_RANK[PLAYER]) {
			// Add all possible promotion options
			for (Piece piece = KNIGHT; piece <= QUEEN; piece++) {
				Move move = create_promotion_move(origin, first, PAWN, piece);
				quasilegal_moves.push_back(move);
			}
		}
		else {
			Move move = create_move(origin, first, PAWN);
			quasilegal_moves.push_back(move);
		}
	}
	// Double push
	Bitboard DOUBLE = shift<FORWARD>(PUSHES) & UNOCCUPIED & Magic::MIDDLE_RANK[PLAYER];
	while (DOUBLE) {
		int first = pop_lsb(DOUBLE);
		int fromSquare = first - (2 * Magic::PAWN_PUSH_AMOUNT[PLAYER]);
		Move move = create_move(fromSquare, first, PAWN, fromSquare + Magic::PAWN_PUSH_AMOUNT[PLAYER]);
		quasilegal_moves.push_back(move);
	}
	
	// Bishops
	Bitboard B = PIECES[BISHOP] & PLAYERS[PLAYER];
	while (B) {
		int origin = pop_lsb(B);
		Bitboard SPAN = diagonal_span(origin);
		if constexpr (Variants::has_friendly_fire_enabled(V))
			SPAN &= ~FRIENDLY_KINGS;
		else
			SPAN &= NON_FRIENDLY;
		while (SPAN) {
			int destination = pop_lsb(SPAN);
			Move move = create_capture_move(origin, destination, BISHOP, list[destination], color_at_square(square_to_bitboard(destination)));
			quasilegal_moves.push_back(move);
//			if (list[destination])
//				captures.push_back(move);
		}
	}
	
	// Knights
	Bitboard N = PIECES[KNIGHT] & PLAYERS[PLAYER];
	while (N) {
		int origin = pop_lsb(N);
		Bitboard SPAN = knight_span(origin);
		if constexpr (Variants::has_friendly_fire_enabled(V))
			SPAN &= ~FRIENDLY_KINGS;
		else
			SPAN &= NON_FRIENDLY;
		while (SPAN) {
			int destination = pop_lsb(SPAN);
			Move move = create_capture_move(origin, destination, KNIGHT, list[destination], color_at_square(square_to_bitboard(destination)));
			quasilegal_moves.push_back(move);
//			if (list[destination])
//				captures.push_back(move);
		}
	}
	
	// Rooks
	Bitboard R = PIECES[ROOK] & PLAYERS[PLAYER];
	while (R) {
		int origin = pop_lsb(R);
		Bitboard SPAN = horizontal_vertical_span(origin);
		if constexpr (Variants::has_friendly_fire_enabled(V))
			SPAN &= ~FRIENDLY_KINGS;
		else
			SPAN &= NON_FRIENDLY;
		while (SPAN) {
			int destination = pop_lsb(SPAN);
			Move move = create_capture_move(origin, destination, ROOK, list[destination], color_at_square(square_to_bitboard(destination)));
			quasilegal_moves.push_back(move);
//			if (list[destination])
//				captures.push_back(move);
		}
	}
	
	// Queens
	Bitboard Q = PIECES[QUEEN] & PLAYERS[PLAYER];
	while (Q) {
		int origin = pop_lsb(Q);
		Bitboard SPAN = diagonal_span(origin) | horizontal_vertical_span(origin);
		if constexpr (Variants::has_friendly_fire_enabled(V))
			SPAN &= ~FRIENDLY_KINGS;
		else
			SPAN &= NON_FRIENDLY;
		while (SPAN) {
			int destination = pop_lsb(SPAN);
			Move move = create_capture_move(origin, destination, QUEEN, list[destination], color_at_square(square_to_bitboard(destination)));
			quasilegal_moves.push_back(move);
//			if (list[destination])
//				captures.push_back(move);
		}
	}
	
	// Castling
	Bitboard KINGS = FRIENDLY_KINGS;
	const Bitboard ATTACKED = attacked_squares<OPPONENT>();
	const Bitboard EMPTY_AND_SAFE = UNOCCUPIED & ~ATTACKED;
	if (can_castle_kingside(PLAYER) && (ATTACKED & KINGS) == 0) {
		if (PLAYER == WHITE) {
			if ((EMPTY_AND_SAFE & square_to_bitboard(5)) && (EMPTY_AND_SAFE & square_to_bitboard(6))) {
				Move move = create_move(4, 6, KING);
				quasilegal_moves.push_back(move);
			}
		}
		else {
			if ((EMPTY_AND_SAFE & square_to_bitboard(61)) && (EMPTY_AND_SAFE & square_to_bitboard(62))) {
				Move move = create_move(60, 62, KING);
				quasilegal_moves.push_back(move);
			}
		}
	}
	if (can_castle_queenside(PLAYER) && (ATTACKED & KINGS) == 0) {
		if (PLAYER == WHITE) {
			if ((EMPTY_AND_SAFE & square_to_bitboard(3)) && (EMPTY_AND_SAFE & square_to_bitboard(2)) && (UNOCCUPIED & square_to_bitboard(1))) {
				Move move = create_move(4, 2, KING);
				quasilegal_moves.push_back(move);
			}
		}
		else {
			if ((EMPTY_AND_SAFE & square_to_bitboard(59)) && (EMPTY_AND_SAFE & square_to_bitboard(58)) && (UNOCCUPIED & square_to_bitboard(57))) {
				Move move = create_move(60, 58, KING);
				quasilegal_moves.push_back(move);
			}
		}
	}
	
	// King
	while (KINGS) {
		int origin = pop_lsb(KINGS);
		Bitboard SPAN = king_span(origin);
		if constexpr (Variants::has_friendly_fire_enabled(V))
			SPAN &= ~FRIENDLY_KINGS;
		else
			SPAN &= NON_FRIENDLY;
		while (SPAN) {
			int destination = pop_lsb(SPAN);
			Move move = create_capture_move(origin, destination, KING, list[destination], color_at_square(square_to_bitboard(destination)));
			quasilegal_moves.push_back(move);
//			if (list[destination])
//				captures.push_back(move);
		}
	}
}

template<Variant V>
inline void Game<V>::generate_quasilegal_moves() const
{
	if (active_player == WHITE)
		generate_quasilegal_moves_for<WHITE>();
	else
		generate_quasilegal_moves_for<BLACK>();
}

template<Variant V>
inline std::vector<Move> Game<V>::legal_moves()
{
	std::vector<Move> moves;
	generate_quasilegal_moves();
	moves.reserve(quasilegal_moves.size());
	
	// Forced capture
	if constexpr (Variants::has_forced_capture_enabled(V)) {
		bool has_legal_capture = false;
		// Run through captures first
		for (Move move : quasilegal_moves) {
			if (move_captured_piece(move) && attempt(move)) {
				has_legal_capture = true;
				moves.push_back(move);
				undo();
			}
		}
		if (has_legal_capture)
			return moves;
		// Add the non-capture moves
		for (Move move : quasilegal_moves) {
			if (move_captured_piece(move) == EMPTY && attempt(move)) {
				moves.push_back(move);
				undo();
			}
		}
	}
	
	// Forced check
	else if constexpr (Variants::has_forced_check_enabled(V)) {
		bool has_check = false;
		std::vector<Move> check_moves;
		check_moves.reserve(8);
		for (Move move : quasilegal_moves) {
			if (attempt(move)) {
				// If this is a check move
				if (is_check(active_player)) {
					has_check = true;
					check_moves.push_back(move);
				}
				moves.push_back(move);
				undo();
			}
		}
		if (has_check)
			return check_moves;
	}
	
	// No special rules
	else {
		for (Move move : quasilegal_moves) {
			if (attempt(move)) {
				moves.push_back(move);
				undo();
			}
		}
	}
	return moves;
}

template<Variant V>
template<Color PLAYER>
inline Bitboard Game<V>::attacked_squares() const
{
	constexpr Direction FORWARD_EAST = (PLAYER == WHITE ? NORTH_EAST : SOUTH_EAST);
	constexpr Direction FORWARD_WEST = (PLAYER == WHITE ? NORTH_WEST : SOUTH_WEST);
	
	Bitboard ATTACKED = 0;
	
	// Pawn capture right
	const Bitboard PAWNS = PIECES[PAWN] & PLAYERS[PLAYER];
	ATTACKED |= shift<FORWARD_EAST>(PAWNS);
	// Pawn capture left
	ATTACKED |= shift<FORWARD_WEST>(PAWNS);
	
	// Knights
	Bitboard N = PIECES[KNIGHT] & PLAYERS[PLAYER];
	while (N) {
		int first = pop_lsb(N);
		ATTACKED |= knight_span(first);
	}
	
	// Bishops & queens
	Bitboard B = (PIECES[BISHOP] | PIECES[QUEEN]) & PLAYERS[PLAYER];
	while (B) {
		int first = pop_lsb(B);
		ATTACKED |= diagonal_span(first);
	}
	
	// Rooks & queens
	Bitboard R = (PIECES[ROOK] | PIECES[QUEEN]) & PLAYERS[PLAYER];
	while (R) {
		int first = pop_lsb(R);
		ATTACKED |= horizontal_vertical_span(first);
	}
	
	// Kings
	Bitboard K = PIECES[KING] & PLAYERS[PLAYER];
	while (K) {
		int first = pop_lsb(K);
		ATTACKED |= king_span(first);
	}
	
	return ATTACKED;
}


// MARK: - Check

template<Variant V>
inline bool Game<V>::is_check(Color player) const
{
	if constexpr (Variants::has_check_disabled(V))
		return false;
	return PIECES[KING] & PLAYERS[player] & (player == WHITE ? attacked_squares<BLACK>() : attacked_squares<WHITE>());
}

template<Variant V>
inline bool Game<V>::is_checkmate()
{
	return is_check(active_player) && is_finished();
}

template<Variant V>
inline bool Game<V>::is_alternative_winning_condition_met(Color player) const
{
	if constexpr (Variants::has_win_by_king_capture(V)) {
		return !(PIECES[KING] & PLAYERS[!player]);
	}
	if constexpr (V == LOSER) {
		return player == active_player && PLAYERS[player] == 0;
	}
	if constexpr (Variants::has_king_of_the_hill(V)) {
		return Magic::CENTER_FOUR_SQUARES & PIECES[KING] & PLAYERS[player];
	}
	return false;
}

template<Variant V>
inline bool Game<V>::is_win(Color player)
{
	if (is_alternative_winning_condition_met(player))
		return true;
	if (player == !active_player && is_checkmate())
		return true;
	return false;
}

template<Variant V>
inline bool Game<V>::is_stalemate()
{
	return !is_check(active_player) && legal_moves().empty();
}

template<Variant V>
inline bool Game<V>::is_draw()
{
	return is_stalemate() || is_fifty_move_draw() || is_three_move_repetition();
}

template<Variant V>
inline bool Game<V>::is_finished()
{
	return legal_moves().empty() || is_alternative_winning_condition_met(WHITE) || is_alternative_winning_condition_met(BLACK) || is_fifty_move_draw() || is_three_move_repetition();
}

template<Variant V>
inline bool Game<V>::is_two_move_repetition() const
{
	// TODO: CHECK WHETHER THIS IS FASTER
//	if (reversible_move_clock < 4)
//		return false;
	
	for (int index = (int)hash_history.size(); index --> hash_history.size() - reversible_move_clock;) {
		if (hash_history[index] == hash)
			return true;
	}
	return false;
}

template<Variant V>
inline bool Game<V>::is_three_move_repetition() const
{
	// TODO: CHECK WHETHER THIS IS FASTER
//	if (reversible_move_clock < 8)
//		return false;
	
	// TODO: CHECK WHETHER CHANGING repetition_count TO A bool IS FASTER
	int repetition_count = 1;
	for (int index = (int)hash_history.size(); index --> hash_history.size() - reversible_move_clock;) {
		if (hash_history[index] == hash) {
			repetition_count++;
			if (repetition_count == 3)
				return true;
		}
	}
	return false;
}

template<Variant V>
inline bool Game<V>::is_fifty_move_draw() const
{
	return fifty_move_rule_enabled && reversible_move_clock >= 75;
}


// MARK: - Apply & Undo

template<Variant V>
inline void Game<V>::apply(Move move)
{
	// History
	move_history.push_back(move);
	EN_PASSANT_HISTORY.push_back(EN_PASSANT);
	castling_rights_history.push_back(castling_rights);
	hash_history.push_back(hash);
	reversible_move_clock_history.push_back(reversible_move_clock);
	
	const int from = move_from(move);
	const int to = move_to(move);
	const Piece piece = move_piece(move);
	const Piece promotion = move_promotion(move);
	const Piece captured_piece = move_captured_piece(move);
	const int enpassant_square = move_enpassant_square(move);
	const Bitboard FROM = square_to_bitboard(from);
	const Bitboard TO = square_to_bitboard(to);
	
	Color captured_piece_color;
	if constexpr (Variants::has_friendly_fire_enabled(V))
		captured_piece_color = move_captured_piece_color(move);
	else
		captured_piece_color = !active_player;
	
	if constexpr (Variants::has_destructive_moves(V)) {
		if (V == EXPLODING_KNIGHTS && piece == KNIGHT && captured_piece) {
			// This move is destructive
			PIECES_HISTORY.emplace_back();
			PLAYERS_HISTORY.emplace_back();
			list_history.emplace_back();
			std::copy(PIECES, PIECES + PIECES_HISTORY.back().size(), PIECES_HISTORY.back().begin());
			std::copy(PLAYERS, PLAYERS + PLAYERS_HISTORY.back().size(), PLAYERS_HISTORY.back().begin());
			std::copy(list, list + list_history.back().size(), list_history.back().begin());
		}
	}
	
//	cout << "APPLYING MOVE:" << endl;
//	cout << "from: " << Notation::square_to_string(from) << endl;
//	cout << "to: " << Notation::square_to_string(to) << endl;
//	cout << "piece: " << piece << endl;
//	cout << "captured_piece: " << captured_piece << endl;
//	cout << "promotion: " << promotion << endl;
//	cout << "CURRENT ENPASSANT: " << Notation::square_to_string(lsb(EN_PASSANT)) << endl;
	
	// From
	PIECES[piece] &= ~FROM;
	PLAYERS[active_player] &= ~FROM;
	hash ^= Zobrist::keys[from][active_player][piece];
	// Captured piece
	PIECES[captured_piece] &= ~TO;
	PLAYERS[captured_piece_color] &= ~TO;
	hash ^= Zobrist::keys[to][captured_piece_color][captured_piece];
	// To
	PIECES[promotion] |= TO;
	PLAYERS[active_player] |= TO;
	hash ^= Zobrist::keys[to][active_player][promotion];
	
	// Piece list
	list[from] = EMPTY;
	list[to] = promotion;
	
	// Explosions
	if constexpr (V == EXPLODING_KNIGHTS) {
		if (piece == KNIGHT && captured_piece) {
			Bitboard BLAST_AREA = (adjacent_squares(to) | TO) & OCCUPIED;
			while (BLAST_AREA) {
				int square = pop_lsb(BLAST_AREA);
				Bitboard SQUARE = square_to_bitboard(square);
				Piece exploded_piece = list[square];
				Color exploded_piece_color = color_at_square(SQUARE);
				PIECES[exploded_piece] &= ~SQUARE;
				PLAYERS[exploded_piece_color] &= ~SQUARE;
				hash ^= Zobrist::keys[square][exploded_piece_color][exploded_piece];
				list[square] = EMPTY;
			}
		}
	}
	
	// En passant
	if (piece == PAWN && (EN_PASSANT & TO)) {
		// Delete enemy pawn
		int captured_pawn;
		if (to > from) {
			// Our pawn pushed up
			captured_pawn = to - 8;
		}
		else {
			// Our pawn pushed down
			captured_pawn = to + 8;
		}
		const Bitboard CAPTURED_PAWN = square_to_bitboard(captured_pawn);
		PIECES[PAWN] &= ~CAPTURED_PAWN;
		PLAYERS[captured_piece_color] &= ~CAPTURED_PAWN;
		list[captured_pawn] = EMPTY;
		hash ^= Zobrist::keys[captured_pawn][captured_piece_color][PAWN];
	}
	
	// En passant
	if (EN_PASSANT)
		hash ^= Zobrist::enpassant_keys[lsb(EN_PASSANT) % 8];
	if (enpassant_square) {
		EN_PASSANT = square_to_bitboard(enpassant_square);
		hash ^= Zobrist::enpassant_keys[enpassant_square % 8];
	}
	else
		EN_PASSANT = 0;
	
	// Castling
	if (piece == KING) {
		if (to - from == 2) {
			// Kingside castling
			PIECES[ROOK] &= ~(FROM << 3);
			PLAYERS[active_player] &= ~(FROM << 3);
			PIECES[ROOK] |= (FROM << 1);
			PLAYERS[active_player] |= (FROM << 1);
			list[from + 3] = EMPTY;
			list[from + 1] = ROOK;
			hash ^= Zobrist::keys[from + 3][active_player][ROOK];
			hash ^= Zobrist::keys[from + 1][active_player][ROOK];
		}
		else if (from - to == 2) {
			// Queenside castling
			PIECES[ROOK] &= ~(FROM >> 4);
			PLAYERS[active_player] &= ~(FROM >> 4);
			PIECES[ROOK] |= (FROM >> 1);
			PLAYERS[active_player] |= (FROM >> 1);
			list[from - 4] = EMPTY;
			list[from - 1] = ROOK;
			hash ^= Zobrist::keys[from - 4][active_player][ROOK];
			hash ^= Zobrist::keys[from - 1][active_player][ROOK];
		}
		if (can_castle_kingside(active_player))
			remove_kingside_castling_right(active_player);
		if (can_castle_queenside(active_player))
			remove_queenside_castling_right(active_player);
	}
	
	// Remove castling rights
	// TODO: FIGURE OUT WHAT SYSTEM IS FASTEST
	if (can_castle_kingside(WHITE) && (
			(PIECES[KING] & PLAYERS[WHITE] & square_to_bitboard(E1)) == 0 ||
			(PIECES[ROOK] & PLAYERS[WHITE] & square_to_bitboard(H1)) == 0
		)) {
		remove_kingside_castling_right(WHITE);
	}
	if (can_castle_queenside(WHITE) && (
			(PIECES[KING] & PLAYERS[WHITE] & square_to_bitboard(E1)) == 0 ||
			(PIECES[ROOK] & PLAYERS[WHITE] & square_to_bitboard(A1)) == 0
		)) {
		remove_queenside_castling_right(WHITE);
	}
	if (can_castle_kingside(BLACK) && (
			(PIECES[KING] & PLAYERS[BLACK] & square_to_bitboard(E8)) == 0 ||
			(PIECES[ROOK] & PLAYERS[BLACK] & square_to_bitboard(H8)) == 0
		)) {
		remove_kingside_castling_right(BLACK);
	}
	if (can_castle_queenside(BLACK) && (
			(PIECES[KING] & PLAYERS[BLACK] & square_to_bitboard(E8)) == 0 ||
			(PIECES[ROOK] & PLAYERS[BLACK] & square_to_bitboard(A8)) == 0
		)) {
		remove_queenside_castling_right(BLACK);
	}
//	if (can_castle_kingside(WHITE) && (PIECES[ROOK] & PLAYERS[WHITE] & square_to_bitboard(H1)) == 0)
//		remove_kingside_castling_right(WHITE);
//	if (can_castle_queenside(WHITE) && (PIECES[ROOK] & PLAYERS[WHITE] & square_to_bitboard(A1)) == 0)
//		remove_queenside_castling_right(WHITE);
//	if (can_castle_kingside(BLACK) && (PIECES[ROOK] & PLAYERS[BLACK] & square_to_bitboard(H8)) == 0)
//		remove_kingside_castling_right(BLACK);
//	if (can_castle_queenside(BLACK) && (PIECES[ROOK] & PLAYERS[BLACK] & square_to_bitboard(A8)) == 0)
//		remove_queenside_castling_right(BLACK);
	
	OCCUPIED = PLAYERS[WHITE] | PLAYERS[BLACK];
	
	active_player = !active_player;
	hash ^= Zobrist::active_player_key;
	
//	repetition_count_current = ++repetition_count[hash];
//	all_previous_positions.insert(hash);
	if (is_irreversible(move))
		reversible_move_clock = 0;
	else
		reversible_move_clock++;
}

template<Variant V>
inline void Game<V>::undo()
{
	// History
	const Move move = move_history.back(); move_history.pop_back();
	EN_PASSANT = EN_PASSANT_HISTORY.back(); EN_PASSANT_HISTORY.pop_back();
	castling_rights = castling_rights_history.back(); castling_rights_history.pop_back();
	hash = hash_history.back(); hash_history.pop_back();
	reversible_move_clock = reversible_move_clock_history.back(); reversible_move_clock_history.pop_back();
	
	active_player = !active_player;
	
	const int from = move_from(move);
	const int to = move_to(move);
	const Piece piece = move_piece(move);
	const Piece captured_piece = move_captured_piece(move);
	const Piece promotion = move_promotion(move);
	const Bitboard FROM = square_to_bitboard(from);
	const Bitboard TO = square_to_bitboard(to);
	
	Color captured_piece_color;
	if constexpr (Variants::has_friendly_fire_enabled(V))
		captured_piece_color = move_captured_piece_color(move);
	else
		captured_piece_color = !active_player;
	
	bool is_destructive = false;
	if constexpr (Variants::has_destructive_moves(V)) {
		if (V == EXPLODING_KNIGHTS && piece == KNIGHT && captured_piece)
			is_destructive = true;
	}
	
	if (is_destructive) {
		// Everything is saved in our history vectors
		std::copy(PIECES_HISTORY.back().begin(), PIECES_HISTORY.back().end(), PIECES);
		std::copy(PLAYERS_HISTORY.back().begin(), PLAYERS_HISTORY.back().end(), PLAYERS);
		std::copy(list_history.back().begin(), list_history.back().end(), list);
		PIECES_HISTORY.pop_back();
		PLAYERS_HISTORY.pop_back();
		list_history.pop_back();
	}
	else {
		
		// Manually undo
		
		// Note: do not use XOR to change `PIECE[move.piece]` because the piece might have promoted
		
		PIECES[promotion] &= ~TO;
		PLAYERS[active_player] &= ~TO;
		PIECES[piece] |= FROM;
		PLAYERS[active_player] |= FROM;
		if (captured_piece) {
			PIECES[captured_piece] |= TO;
			PLAYERS[captured_piece_color] |= TO;
		}
		
		// Piece list
		list[from] = piece;
		list[to] = captured_piece;
		
		// Castling
		if (piece == KING) {
			if (to - from == 2) {
				// Kingside castling
				PIECES[ROOK] |= FROM << 3;
				PLAYERS[active_player] |= FROM << 3;
				PIECES[ROOK] &= ~(FROM << 1);
				PLAYERS[active_player] &= ~(FROM << 1);
				list[from + 3] = ROOK;
				list[from + 1] = EMPTY;
			}
			else if (from - to == 2) {
				// Queenside castling
				PIECES[ROOK] |= FROM >> 4;
				PLAYERS[active_player] |= FROM >> 4;
				PIECES[ROOK] &= ~(FROM >> 1);
				PLAYERS[active_player] &= ~(FROM >> 1);
				list[from - 4] = ROOK;
				list[from - 1] = EMPTY;
			}
		}
		
		// Decrement the repetition count and remove the entry if the count is now zero
//		if (! --repetition_count[hash])
//			repetition_count.erase(hash);
//		all_previous_positions.erase(all_previous_positions.find(hash));
		
//		repetition_count_current = repetition_count[hash];
		
		// En passant
		if (piece == PAWN && (EN_PASSANT & TO)) {
			// Restore enemy pawn
			int enemy_pawn;
			if (to > from) {
				// Our pawn pushed up
				enemy_pawn = to - 8;
			}
			else {
				// Our pawn pushed down
				enemy_pawn = to + 8;
			}
			const Bitboard ENEMY_PAWN = square_to_bitboard(enemy_pawn);
			PIECES[PAWN] |= ENEMY_PAWN;
			PLAYERS[captured_piece_color] |= ENEMY_PAWN;
			list[enemy_pawn] = PAWN;
		}
	}
	
	OCCUPIED = PLAYERS[WHITE] | PLAYERS[BLACK];
}

template<Variant V>
inline bool Game<V>::attempt(Move move)
{
	apply(move);
	if (is_check(!active_player)) {
		undo();
		return false;
	}
	return true;
}


// MARK: - Utilities

template<Variant V>
inline Color Game<V>::color_at_square(Bitboard B) const
{
	return PLAYERS[BLACK] & B;
}

template<Variant V>
inline Color Game<V>::can_castle_kingside(Color player) const
{
	return castling_rights & CASTLE_KINGSIDE[player];
}

template<Variant V>
inline Color Game<V>::can_castle_queenside(Color player) const
{
	return castling_rights & CASTLE_QUEENSIDE[player];
}

template<Variant V>
inline void Game<V>::remove_kingside_castling_right(Color player)
{
	castling_rights &= ~CASTLE_KINGSIDE[player];
	hash ^= Zobrist::kingside_castling_keys[player];
}

template<Variant V>
inline void Game<V>::remove_queenside_castling_right(Color player)
{
	castling_rights &= ~CASTLE_QUEENSIDE[player];
	hash ^= Zobrist::queenside_castling_keys[player];
}


// MARK: - SPAN

template<Variant V>
inline Bitboard Game<V>::adjacent_squares(int square) const
{
	return ADJACENT_SQUARES[square];
}

template<Variant V>
inline Bitboard Game<V>::horizontal_vertical_span(int square) const
{
	// Horizontal
	const Bitboard O_HORIZONTAL = (OCCUPIED >> Magic::START_OF_RANK[square]) & Bitboards::RANK_1;
	const Bitboard HORIZONTAL = Magic::HORIZONTAL_SPAN[square][O_HORIZONTAL];
	
	// Vertical
	const Bitboard O_VERTICAL = ((OCCUPIED & FILES[square]) * Magic::VERTICAL_MULTIPLICAND[square]) >> A8;
	const Bitboard VERTICAL = Magic::VERTICAL_SPAN[square][O_VERTICAL];
	
	return HORIZONTAL | VERTICAL;
}

template<Variant V>
inline Bitboard Game<V>::diagonal_span(int square) const
{
	// Diagonal
	const Bitboard O_DIAGONAL = ((OCCUPIED & DIAGONALS[square]) * Magic::DIAGONAL_MULTIPLICAND[square]) >> A8;
	const Bitboard DIAGONAL = Magic::DIAGONAL_SPAN[square][O_DIAGONAL];
	
	// Anti-diagonal
	const Bitboard O_ANTI_DIAGONAL = ((OCCUPIED & ANTI_DIAGONALS[square]) * Magic::ANTI_DIAGONAL_MULTIPLICAND[square]) >> A8;
	const Bitboard ANTI_DIAGONAL = Magic::ANTI_DIAGONAL_SPAN[square][O_ANTI_DIAGONAL];
	
	return DIAGONAL | ANTI_DIAGONAL;
}

template<Variant V>
inline Bitboard Game<V>::knight_span(int square) const
{
	return KNIGHT_SPAN[square];
}

template<Variant V>
inline Bitboard Game<V>::king_span(int square) const
{
	return adjacent_squares(square);
}

#endif /* game_h */

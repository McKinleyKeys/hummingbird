//
//  hummingbird.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#pragma once
#ifndef hummingbird_h
#define hummingbird_h

#include "fruit.h"
#include "game.h"
#include "definitions.h"
#include "table.h"
#include "opening_book.h"

class AbstractHummingbird
{
public:
	
	/// Returns a pointer to a new instance of `Hummingbird<variant>`.
	static std::unique_ptr<AbstractHummingbird> instantiate(Variant variant);
	
	virtual ~AbstractHummingbird() = default;
	
	virtual void load_opening_book(const std::string &book_name) = 0;
	/// Loads the default opening book for this variant, if one exists.
	virtual void load_default_opening_book() = 0;
	
	virtual void setup(const AbstractGame &new_game) = 0;
	virtual void apply(Move move) = 0;
	
	virtual void default_setup() = 0;
	virtual void setup_fen(const std::string &fen) = 0;
	virtual void setup_visual(const std::string &visual) = 0;
	
	virtual Move find_best_move(int depth) = 0;
	virtual Move find_best_move(int depth, double seconds) = 0;
};

template<Variant V>
class Hummingbird: public AbstractHummingbird
{
public:
	
	Game<V> game;
	
	uint64_t node_count = 0;
	uint64_t leaf_node_count = 0;
	
	static constexpr int depth_limit = 9;
	static constexpr bool uses_opening_book = true;
	OpeningBook opening_book;
	
	Table<HummingbirdEntry> table{10'000'000};
	bool table_is_empty = true;
	
	static constexpr int CHECKMATE_SCORE = 1'000'000;
	
protected:
	
	bool searching = false;
	int max_depth = 0;
	int searches_finished = 0;
	fruit::DispatchQueue background_queue{"com.mckinleykeys.chaos-chess.hummingbird.hummingbird-background-queue"};
	
public:
	
	Hummingbird();
	
	
	// MARK: - Searching
	
	Move find_best_move(int depth);
	Move find_best_move(int depth, double seconds);
	std::pair<int, Move> search(int depth, int alpha, int beta, Move hint);
	
	inline void sort_moves(const std::vector<Move> &moves, std::vector<std::pair<int, Move>> &ordered_moves) const
	{
		ordered_moves.reserve(moves.size());
		for (Move move : moves) {
			
			int value = 0;
			value += Magic::PIECE_SCORES[false][game.active_player][move_piece(move)][move_to(move)];
			value -= Magic::PIECE_SCORES[false][game.active_player][move_piece(move)][move_from(move)];
			value += Magic::PIECE_SCORES[false][!game.active_player][move_captured_piece(move)][move_to(move)];
			
			ordered_moves.emplace_back(-value, move);
		}
		std::sort(ordered_moves.begin(), ordered_moves.end());
	}
	
	void stop_after(double seconds);
	void stop_immediately();
	
	
	// MARK: - Evaluation
	
	inline int evaluate(int depth) const
	{
		int material_middlegame[] = {0, 0};
		int material_endgame[] = {0, 0};
		int endgame_progress = 0;
		
		int total = 0;
		for (int player = WHITE; player <= BLACK; player++) {
			
			int score = 0;
			
			// Material
			Bitboard FRIENDLY = game.PLAYERS[player];
			while (FRIENDLY) {
				int square = pop_lsb(FRIENDLY);
				Piece piece = game.list[square];
				material_middlegame[player] += Magic::PIECE_SCORES[false][player][piece][square];
				material_endgame[player] += Magic::PIECE_SCORES[true][player][piece][square];
				endgame_progress += Magic::ENDGAME_PROGRESS[piece];
			}
			
			// Bishops
			if (popcount(game.PIECES[BISHOP] & game.PLAYERS[player]) >= 2)
				score += 30;
			
			// Moveability
			if (player == WHITE)
				game.template generate_quasilegal_moves_for<WHITE>();
			if (player == BLACK)
				game.template generate_quasilegal_moves_for<BLACK>();
			size_t move_count = game.quasilegal_moves.size();
			score += 4 * (int)move_count;
			
			// Attacks
			Bitboard ATTACKED_FRIENDLY = game.PLAYERS[player] & ((player == WHITE) ? game.template attacked_squares<BLACK>() : game.template attacked_squares<WHITE>());
			score -= 8 * popcount(game.PIECES[PAWN] & ATTACKED_FRIENDLY);
			score -= 40 * popcount(game.PIECES[KNIGHT] & ATTACKED_FRIENDLY);
			score -= 40 * popcount(game.PIECES[BISHOP] & ATTACKED_FRIENDLY);
			score -= 80 * popcount(game.PIECES[ROOK] & ATTACKED_FRIENDLY);
			score -= 120 * popcount(game.PIECES[QUEEN] & ATTACKED_FRIENDLY);
			score -= 220 * popcount(game.PIECES[KING] & ATTACKED_FRIENDLY);
			
			// Castling rights
			if constexpr (V == KING_OF_THE_HILL_AND_COMPULSION) {
				// Castling rights don't matter
			}
			else {
				if (game.can_castle_kingside(player))
					score += 20;
				if (game.can_castle_queenside(player))
					score += 20;
			}
			
			// Variant-specific factors
			if constexpr (V == KING_OF_THE_HILL_AND_COMPULSION) {
				Bitboard K = game.PIECES[KING] & game.PLAYERS[player];
				if (K & Magic::RING_OF_RADIUS_2)
					score += 400;
				else if (K & Magic::RING_OF_RADIUS_3)
					score += 200;
			}

			if (player == game.active_player) total += score;
			else total -= score;
		}
		
		// Material
		endgame_progress = std::min(endgame_progress, 24);
		const int total_material = ((material_middlegame[game.active_player] - material_middlegame[!game.active_player]) * endgame_progress + (material_endgame[game.active_player] - material_endgame[!game.active_player]) * (24 - endgame_progress)) / 24;
		if constexpr (V == LOSER) {
			total -= total_material;
		}
		else {
			total += total_material;
		}
		
		return total;
	}
	
	inline int checkmate_score(int depth) const
	{
		return CHECKMATE_SCORE - depth;
	}
	
	
	// MARK: - Configuration
	
	void load_opening_book(const std::string &book_name);
	/// Loads the default opening book for this variant, if one exists.
	void load_default_opening_book();
	
	void setup(const AbstractGame &new_game);
	void apply(Move move);
	
	void default_setup();
	void setup_fen(const std::string &fen);
	void setup_visual(const std::string &visual);
};

#endif /* hummingbird_h */

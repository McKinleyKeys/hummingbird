//
//  hummingbird.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#include "hummingbird.h"

template<Variant V>
Hummingbird<V>::Hummingbird() = default;

std::unique_ptr<AbstractHummingbird> AbstractHummingbird::instantiate(Variant variant)
{
	switch (variant) {
		#define CASE_STATEMENT_FOR_VARIANT(V) \
			case V: \
				return std::make_unique<Hummingbird<V>>(); \
				break;
		FOR_EACH_VARIANT(CASE_STATEMENT_FOR_VARIANT)
		#undef CASE_STATEMENT_FOR_VARIANT
		default:
			return nullptr;
	}
}

constexpr int INF = INT_MAX;


// MARK: - Searching

template<Variant V>
Move Hummingbird<V>::find_best_move(int depth)
{
	fruit::Stopwatch stopwatch;
	stopwatch.start();
	searching = true;
	
	// Check whether the game is finished
	if (game.is_finished()) {
		
		cout << "(Warning) the game is finished according to Hummingbird's rule set";
		std::vector<std::string> reasons;
		if (game.is_fifty_move_draw())
			reasons.push_back("fifty move rule");
		if (game.is_three_move_repetition())
			reasons.push_back("three move repetition");
		if (game.is_checkmate())
			reasons.push_back("checkmate");
		if (game.is_alternative_winning_condition_met(WHITE))
			reasons.push_back("alternative winning condition met for white");
		if (game.is_alternative_winning_condition_met(BLACK))
			reasons.push_back("alternative winning condition met for black");
		if (game.is_stalemate())
			reasons.push_back("stalemate");
		if (reasons.size())
			cout << " (" << fruit::join(reasons, ", ") << ")";
		cout << endl;
		
		// Return any legal move
		const std::vector<Move> moves = game.legal_moves();
		if (moves.empty())
			return NULL_MOVE;
		std::vector<std::pair<int, Move>> ordered_moves;
		sort_moves(moves, ordered_moves);
		// Return the move that looks best
		return ordered_moves.front().second;
	}
	
	// Opening book
	if (uses_opening_book) {
		
		if (!opening_book.loaded)
			cout << "(Warning) Hummingbird has no opening book" << endl;
		
		Move move = opening_book.random_move(game);
		if (move) {
			searching = false;
			searches_finished++;
			return move;
		}
	}
	
	table_is_empty = false;
	
	if (!searching) {
		cout << "Exiting early because searching is false" << endl;
		exit(3);
	}
	
	Move previous_best_move = NULL_MOVE;
	int current_depth = 1;
	do {
		
		if (depth > 0 && current_depth > depth)
			current_depth = std::min(current_depth, depth);
		
		max_depth = current_depth;
		const std::pair<int, Move> result = search(0, -INF, INF, previous_best_move);
		
		// If the search exited early because it ran out of time, we can't trust the return value
		if (!searching)
			break;
		
		previous_best_move = result.second;
		
		current_depth++;
	}
	while (searching && (current_depth <= depth || depth == 0));
	
	searching = false;
	searches_finished++;
	
	// Print a warning if the move we chose is actually illegal
	if (previous_best_move != NULL_MOVE && !fruit::contains(game.legal_moves(), previous_best_move))
		cout << "(Warning) Hummingbird chose illegal move " << fruit::debug_description(Notation::move_to_string(previous_best_move)) << endl;
	
	return previous_best_move;
}

template<Variant V>
Move Hummingbird<V>::find_best_move(int depth, double seconds)
{
	stop_after(seconds);
	return find_best_move(depth);
}

// TODO: CONSIDER CHANGING RETURN TYPE TO uint64_t
// TODO: PERHAPS ADD template<Color PLAYER>?
template<Variant V>
std::pair<int, Move> Hummingbird<V>::search(int depth, int alpha, int beta, Move hint)
{
	node_count++;
	
	// Check for alternative win
	if constexpr (Variants::has_alternative_winning_condition(V)) {
		if (game.is_alternative_winning_condition_met(game.active_player))
			return { checkmate_score(depth), NULL_MOVE };
		if (game.is_alternative_winning_condition_met(!game.active_player))
			return { -checkmate_score(depth), NULL_MOVE };
	}
	// Check for draw
	if (game.is_fifty_move_draw() || game.is_three_move_repetition()) {
		if (depth == 0) {
			if (game.is_fifty_move_draw())
				cout << "Drawing at depth 0 from fifty move rule" << endl;
			if (game.is_three_move_repetition())
				cout << "Drawing at depth 0 from three-move repetition" << endl;
		}
		return { std::max(alpha, 0), NULL_MOVE };
	}
	
	const int initial_alpha = alpha;
	
	bool has_legal_moves = false;
	Move best_move = NULL_MOVE;
	
	// Look up the position in the transposition table
	Move hash_move = NULL_MOVE;
	bool should_skip_hash_move = false;
	{
		const HummingbirdEntry *entry = table.get(game.hash);
		if (entry && !game.is_two_move_repetition()) {
			
			if (entry->remaining_depth >= max_depth - depth) {
				switch (entry->precision) {
					
					case HummingbirdEntry::EXACT:
						if (depth == 0 && entry->best_move == NULL_MOVE)
							cout << "Returning null at depth 0 from transposition table";
						return { entry->score, entry->best_move };
					
					case HummingbirdEntry::LOWER_BOUND:
//						alpha = std::max(alpha, entry->score);
						if (entry->score > alpha) {
							alpha = entry->score;
							best_move = entry->best_move;
							has_legal_moves = true;
							should_skip_hash_move = true;
						}
						break;
					
					case HummingbirdEntry::UPPER_BOUND:
						beta = std::min(beta, entry->score);
						break;
					
					case HummingbirdEntry::NONE:
						break;
				}
				if (alpha >= beta) {
					if (depth == 0 && entry->best_move == NULL_MOVE)
						cout << "Returning null at depth 0 from transposition table because of beta cutoff" << endl;
					return { alpha, entry->best_move };
				}
			}
			hash_move = entry->best_move;
		}
	}
	
	// Leaf node
	if (depth == max_depth) {
		leaf_node_count++;
		if (depth == 0)
			cout << "Returning null at depth 0 at leaf node" << endl;
		return { evaluate(depth), NULL_MOVE };
	}
//	if (!searching) {
//		return { alpha, 0 };
//	}
	
	bool zero_window = false;
	int score = 0;
	
	Move move_to_play;
	int goto_origin;
	
	std::vector<Move> moves;
	int move_index = 0;
	std::vector<std::pair<int, Move>> ordered_moves;
	
	// Trials
	{
		// Special moves
		if (hint) {
			move_to_play = hint;
			goto_origin = 0;
			goto play_move;
			hint_move_origin:
			if (alpha >= beta)
				goto trials_end;
			if (!searching) {
				if (depth == 0 && best_move == NULL_MOVE)
					cout << "Returning null at depth 0 from hint move" << endl;
				return { alpha, best_move };
			}
		}
		if (hash_move && !should_skip_hash_move) {
			move_to_play = hash_move;
			goto_origin = 1;
			goto play_move;
			hash_move_origin:
			if (alpha >= beta)
				goto trials_end;
			if (!searching) {
				if (depth == 0 && best_move == NULL_MOVE)
					cout << "Returning null at depth 0 from hash move" << endl;
				return { alpha, best_move };
			}
		}
		
		if constexpr (Variants::has_forced_capture_enabled(V) || Variants::has_forced_check_enabled(V)) {
			// These variants have special rules about what moves are legal
			moves = game.legal_moves();
		}
		else {
			game.generate_quasilegal_moves();
			// TODO: AVOID COPYING
			moves = game.quasilegal_moves;
		}
		// Remove `hint` and `hash_move` from `moves`
		if (hint) {
			const auto index = std::find(moves.begin(), moves.end(), hint);
			if (index != moves.end())
				moves.erase(index);
			// TODO: CHECK WHETHER `moves.erase(hint)` is faster
		}
		if (hash_move) {
			const auto index = std::find(moves.begin(), moves.end(), hash_move);
			if (index != moves.end())
				moves.erase(index);
		}
		
		ordered_moves.reserve(moves.size());
		for (Move move : moves) {
			
			int value = 0;
			value += Magic::PIECE_SCORES[false][game.active_player][move_piece(move)][move_to(move)];
			value -= Magic::PIECE_SCORES[false][game.active_player][move_piece(move)][move_from(move)];
			value += Magic::PIECE_SCORES[false][!game.active_player][move_captured_piece(move)][move_to(move)];
			
			ordered_moves.emplace_back(-value, move);
		}
		std::sort(ordered_moves.begin(), ordered_moves.end());
		
		normal_move_origin:
		if (alpha >= beta)
			goto trials_end;
		while (move_index < ordered_moves.size()) {
			
			if (!searching) {
				if (depth == 0 && best_move == NULL_MOVE)
					cout << "Returning null at depth 0 from main search loop" << endl;
				return { alpha, best_move };
			}
			
			move_to_play = ordered_moves[move_index].second;
			goto_origin = 2;
			move_index++;
			goto play_move;
		}
		
		// Check whether the game is finished
		if (!has_legal_moves) {
			if constexpr (Variants::has_win_by_checkmate(V)) {
				if (game.is_check(game.active_player))
					alpha = std::max(alpha, -checkmate_score(depth));
				else
					alpha = std::max(alpha, 0);
			}
			else {
				// This must be a stalemate
				alpha = std::max(alpha, 0);
			}
		}
	}
	
	trials_end:
	
	// Update transposition table
	{
		HummingbirdEntry::Precision precision = HummingbirdEntry::NONE;
		if (best_move) {
			if (alpha >= initial_alpha && alpha < beta) {
				// We found an exact score
				precision = HummingbirdEntry::EXACT;
			}
			else if (alpha >= beta) {
				// We found a lower bound
				precision = HummingbirdEntry::LOWER_BOUND;
			}
		}
		else {
			// All moves returned scores less than `initial_alpha`, meaning `initial_alpha` is an upper bound
			precision = HummingbirdEntry::UPPER_BOUND;
		}
		if (precision != HummingbirdEntry::NONE) {
			HummingbirdEntry entry(game.hash);
			entry.precision = precision;
			entry.score = alpha;
			entry.remaining_depth = max_depth - depth;
			entry.best_move = best_move;
			table.put(entry);
		}
	}
	
	// Fail hard by making sure the return value is in the range `original_alpha ... beta`
	alpha = std::min(alpha, beta);
	
	if (depth == 0 && best_move == NULL_MOVE)
		cout << "Returning null at depth 0 normally" << endl;
	
	return { alpha, best_move };
	
	
	
	play_move:
	{
		bool is_valid_move;
		if constexpr (Variants::has_forced_capture_enabled(V) || Variants::has_forced_check_enabled(V)) {
			// We used `game.legal_moves()` to generate moves above, so `move_to_play` is definitely legal
			is_valid_move = true;
			game.apply(move_to_play);
		}
		else {
			// `move_to_play` is quasilegal, so we have to check whether it is actually legal
			is_valid_move = game.attempt(move_to_play);
		}
		if (is_valid_move) {
			
			has_legal_moves = true;
			
//			(score, _) = _search(game: game, depth: depth + 1, alpha: -beta, beta: -alpha, hint: NULL_MOVE)
//			score = -score
			if (!zero_window) {
				// Search with full window
				const auto result = search(depth + 1, -beta, -alpha, NULL_MOVE);
				score = -result.first;
				zero_window = true;
			}
			else {
				// Try searching with zero-width window
				auto result = search(depth + 1, -(alpha + 1), -alpha, NULL_MOVE);
				score = -result.first;
				const Move zero_window_best_move = result.second;
				if (score > alpha && score < beta) {
					// The real score is in the range `(alpha + 1) ..< beta`. We need to search again with full-width window to find the real score.
					result = search(depth + 1, -beta, -alpha, zero_window_best_move);
					score = -result.first;
					zero_window = false;
				}
			}
			game.undo();
			if (score > alpha) {
				alpha = score;
				best_move = move_to_play;
			}
		}
	}
	
	switch (goto_origin) {
		case 0:
			goto hint_move_origin;
		case 1:
			goto hash_move_origin;
		case 2:
			goto normal_move_origin;
		default:
			fruit::fatal_error("Reached last line of Hummingbird's search function!");
	}
}


template<Variant V>
void Hummingbird<V>::stop_after(double seconds)
{
	auto func = [target_searches_finished = searches_finished, this]() {
		// We only want to stop the search that started right after `stop_after` was called. It is possible that that search ended early and that another search is happening right now, in which case we don't want to stop the other search.
		if (searches_finished == target_searches_finished)
			stop_immediately();
	};
	background_queue.async_after(seconds - 0.005, func);
}
template<Variant V>
void Hummingbird<V>::stop_immediately()
{
	searching = false;
}


// MARK: - Configuration

template<Variant V>
void Hummingbird<V>::load_opening_book(const std::string &book_name)
{
	opening_book.load(book_name);
}
template<Variant V>
void Hummingbird<V>::load_default_opening_book()
{
	load_opening_book(OpeningBooks::default_book_name_for_variant(V));
}

template<Variant V>
void Hummingbird<V>::setup(const AbstractGame &new_abstract_game)
{
	// Try to cast `new_abstract_game` to a `Game<V>`
	const Game<V> *new_game_ptr = dynamic_cast<const Game<V> *>(&new_abstract_game);
//	if (new_game_ptr == nullptr) {
//		std::string variant_name = Notation::variant_to_string(V);
//		cout << "(Warning) tried to set up Hummingbird<" << variant_name << ">" << " with a game that is not a Game<" << variant_name << ">" << endl;
//		return;
//	}
//	const Game<V> &new_game = *new_game_ptr;
//
//	// Decide whether to clear the transposition table
//	if (!table_is_empty) {
//		bool should_reset = false;
//		for (int player = WHITE; player <= BLACK; player++) {
//			// Check whether pawns have moved
//			if ((new_game.PIECES[PAWN] & new_game.PLAYERS[player]) != (game.PIECES[PAWN] & game.PLAYERS[player]))
//				should_reset = true;
//			// Check whether there is a different number of pieces
//			for (Piece piece = KNIGHT; piece <= KING; piece++)
//				if (popcount(new_game.PIECES[piece] & new_game.PLAYERS[player]) != popcount(game.PIECES[piece] & game.PLAYERS[player]))
//					should_reset = true;
//		}
//		if (should_reset) {
//			table.reset();
//			table_is_empty = true;
//		}
//	}
//
//	game = new_game;
}

template<Variant V>
void Hummingbird<V>::apply(Move move)
{
	Game new_game = game;
	new_game.apply(move);
	setup(new_game);
}

template<Variant V>
void Hummingbird<V>::default_setup()
{
	Game<V> new_game;
	new_game.default_setup();
	setup(new_game);
}

template<Variant V>
void Hummingbird<V>::setup_fen(const std::string &fen)
{
	Game<V> new_game;
	new_game.setup_fen(fen);
	setup(new_game);
}

template<Variant V>
void Hummingbird<V>::setup_visual(const std::string &visual)
{
	Game<V> new_game;
	new_game.setup_visual(visual);
	setup(new_game);
}


// Explicitly instantiate the `Hummingbird` class with each variant
//template class Hummingbird<CLASSIC>;
//template class Hummingbird<ENERGY_CELLS>;
//template class Hummingbird<EXPLODING_KNIGHTS>;
//template class Hummingbird<COMPULSION>;
//template class Hummingbird<COMPULSION_AND_BACKSTABBING>;
//template class Hummingbird<PETRIFICATION>;
//template class Hummingbird<FORCED_CHECK>;
//template class Hummingbird<LOSER>;
//template class Hummingbird<KING_OF_THE_HILL>;
//template class Hummingbird<KING_OF_THE_HILL_AND_COMPULSION>;
//template class Hummingbird<THREE_CHECK>;
//template class Hummingbird<FOG_OF_WAR>;

#define INSTANTIATE_HUMMINGBIRD(VARIANT) \
	template class Hummingbird<VARIANT>;

FOR_EACH_VARIANT(INSTANTIATE_HUMMINGBIRD)

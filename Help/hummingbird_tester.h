//
//  hummingbird_tester.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#pragma once
#ifndef hummingbird_tester_h
#define hummingbird_tester_h

#include "fruit.h"
#include "definitions.h"
#include "hummingbird.h"
#include "notation.h"
#include <cmath>

namespace HummingbirdTester
{

void compare_elo(int argc, char **argv);
void calculate_elo(int argc, char **argv);

template<Variant V>
void speed_test()
{
	/*
	 
	 Best move: c2c4
	 
	 Nodes: 1,479,682
	 Nodes per second: 1,327k
	 Total time: 1.11
	 
	 */
	
	const int depth = 7;
	
	cout << "Performing speed test..." << endl;
	
	Hummingbird<V> hummingbird;
	hummingbird.setup_fen("mango");
	hummingbird.game.display();
	
	fruit::Stopwatch stopwatch;
	stopwatch.start();
	
	Move move = hummingbird.find_best_move(depth);
	
	cout << "Best move: " << Notation::move_to_string(move) << endl;
	cout << endl;
	
	double time_taken = stopwatch.check();
	// Print the nodes per second styled nicely
	int nodes_per_second = (int)((double)hummingbird.node_count / time_taken);
	std::string kilonodes_per_second = fruit::thousands_separated_by_commas(nodes_per_second / 1000) + "k";
	cout << "Nodes: " << fruit::thousands_separated_by_commas(hummingbird.node_count) << endl;
	cout << "Nodes per second: " << kilonodes_per_second << endl;
	cout << "Total time: " << std::round(time_taken * 100) / 100 << endl;
}

template<Variant V>
void rigorous_test()
{
	cout << "Performing rigorous speed test..." << endl;
	
	std::vector<std::string> fens = {
		"mango",
		"kiwi",
		"grape",
		"q6k/2r2npp/4Qp2/p7/Pp6/1P1R4/2r3PP/B2R3K w - - 0 1",		// Puzzle 1
	};
	
	const int depth = 7;
	
	double total_time = 0;
	fruit::Stopwatch stopwatch;
	
	Hummingbird<V> hummingbird;
	
	for (std::string fen : fens) {
		
		hummingbird.setup_fen(fen);
		hummingbird.game.display();
		
		stopwatch.start();
		
		Move move = hummingbird.find_best_move(depth);
		cout << "Best move: " << Notation::move_to_string(move) << endl;
		cout << endl;
		
		total_time += stopwatch.check();
	}
	
	// Print the nodes per second styled nicely
	int nodes_per_second = (int)((double)hummingbird.node_count / total_time);
	std::string kilonodes_per_second = fruit::thousands_separated_by_commas(nodes_per_second / 1000) + "k";
	cout << "Nodes: " << fruit::thousands_separated_by_commas(hummingbird.node_count) << endl;
	cout << "Nodes per second: " << kilonodes_per_second << endl;
	cout << "Total time: " << std::round(total_time * 100) / 100 << endl;
}

template<Variant V>
void puzzle_test()
{
	std::vector<std::tuple<std::string, int, std::string, bool>> problems =
	{
		// Rd8+. Depth 5. Checkmate. https://www.chess.com/daily-chess-puzzle/2021-12-24
		{
			R"(
			q . . . . . . k
			. . r . . n p p
			. . . . Q p . .
			p . . . . . . .
			P p . . . . . .
			. P . R . . . .
			. . r . . . P P
			B . . R . . . K
			w - -
			)",
			5,
			"d3d8",
			true
		},
		// Nxd4. Depth ~7. Wins (rook + knight + pawn) for (knight + pawn). https://www.chess.com/daily-chess-puzzle/2021-12-24
		{
			R"(
			. . k r . . . r
			p p p q . . . b
			. . n b . . . p
			. . . p . . . P
			. . . P . n P N
			. Q P . . . B .
			P P . N . P . .
			. . K R . B . R
			b - -
			)",
			7,
			"c6d4",
			false
		},
		// Rd7. Depth ~5. Wins (rook) for (knight). https://www.chess.com/daily-chess-puzzle/2021-12-24
		{
			R"(
			. . k r . . . .
			Q . . . . p . .
			. . q . p P . .
			. p . . . . B .
			. . p . . . . P
			. . . n . . P .
			P . . . . P . .
			. . . R . . K .
			b - -
			)",
			7,
			"d8d7",
			true
		},
		// Qh5. Depth ~9. Wins (queen + pawn) for (rook + pawn). https://www.chess.com/daily-chess-puzzle/2021-12-25
		{
			R"(
			r . . . . . . k
			. p p . . . . r
			. b . . Q p . .
			p . . . . N . .
			P . . P . P . .
			. . . . P K p .
			. . . . . . P q
			. B . . R . . .
			b - -
			)",
			9,
			"h2h5",
			false
		},
		// Rc3+. Depth ~9. Wins (queen) for (rook + knight + pawn). NOTE: TESTS KNOWLEDGE THAT QUEENS HAVE HIGH VALUE IN ENDGAME. https://www.chess.com/daily-chess-puzzle/2021-12-26
		{
			R"(
			. . . Q . . . .
			. . . . . . . .
			. . . p . . . .
			. . . N . . . P
			. . . . . . . P
			. k . . . . . .
			. . . . . . . .
			K . R . . . . .
			w - -
			)",
			9,
			"c1c3",
			false
		},
	};
	
	Hummingbird<V> hummingbird;
	
	for (int a = 0; a < problems.size(); a++) {
		
		auto [visual, rough_depth, correct_move, include] = problems[a];
		if (!include || rough_depth > hummingbird.depth_limit)
			continue;
		
		cout << "------------" << endl;
		cout << "Puzzle " << a + 1 << endl;
		cout << endl;
		
		hummingbird.setup_visual(visual);
		
		fruit::Stopwatch stopwatch;
		stopwatch.start();
		Move move = hummingbird.find_best_move(rough_depth + 1);
		double time_taken = stopwatch.check();
		
		if (Notation::move_to_string(move) == correct_move) {
			cout << "Correct!" << endl;
			// Print the nodes per second styled nicely
			int nodes_per_second = (int)((double)hummingbird.node_count / time_taken);
			std::string kilonodes_per_second = fruit::thousands_separated_by_commas(nodes_per_second / 1000) + "k";
			cout << "Nodes: " << fruit::thousands_separated_by_commas(hummingbird.node_count) << endl;
			cout << "Nodes per second: " << kilonodes_per_second << endl;
			cout << "Total time: " << std::round(time_taken * 100) / 100 << endl;
		}
		else {
			cout << "Incorrect. Hummingbird returned " << Notation::move_to_string(move) << " but the correct move is " << correct_move << endl;
		}
		cout << endl;
	}
}


// MARK: - Endgame

namespace SimpleEndgameAI
{
	static inline int distance(int square_a, int square_b)
	{
		return abs(square_a / 8 - square_b / 8) + abs(square_a % 8 - square_b % 8);
	}
	template<Variant V>
	Move find_best_move(Game<V> &game)
	{
		const Color opponent = !game.active_player;
		
		// Assign each move a score
		std::vector<std::pair<int, Move>> scored_moves;
		for (const Move move : game.legal_moves()) {
			
			int score = INT_MIN;
			if (game.PIECES[ROOK] & game.PLAYERS[opponent]) {
				
				// Try to keep king as close as possible to the center
				switch (move_piece(move)) {
					
					case KING: {
						int to = move_to(move);
						int other_king = lsb(game.PIECES[KING] & game.PLAYERS[opponent]);
						score = -distance(to, D4);
						score += distance(to, other_king);
						break;
					}
					default:
						break;
				}
			}
			
			scored_moves.emplace_back(score, move);
		}
		
		auto index = std::max_element(scored_moves.begin(), scored_moves.end());
		return index->second;
	}
	
	template<Variant V>
	bool is_sufficient_material(const Game<V> &game)
	{
		if (game.PIECES[ROOK] | game.PIECES[QUEEN] | game.PIECES[PAWN])
			return true;
		for (int player = WHITE; player <= BLACK; player++) {
			bool has_knight = game.PIECES[KNIGHT] & game.PLAYERS[player];
			bool has_bishop = game.PIECES[BISHOP] & game.PLAYERS[player];
			if (has_knight && has_bishop)
				return true;
		}
		return false;
	}
}

template<Variant V>
void endgame_test()
{
	std::vector<std::tuple<std::string, int, bool>> problems =
	{
		// King & rook vs. king
		{
			R"(
			. . . . . . . .
			. . . . . . R .
			. . . . . . . .
			. . . . . . . .
			. . . . K . . .
			. . . . . . . .
			. . . . k . . .
			. . . . . . . .
			w - -
			)",
			11,
			true
		},
		// King & rook vs. king
		{
			R"(
			. . . . . . . .
			. . K . . . . .
			. . . . . . . .
			. . . . . . . .
			. . . . . . R .
			. . . . . . . .
			. . . . k . . .
			. . . . . . . .
			w - -
			)",
			30,
			true
		},
	};
	
	Hummingbird<V> hummingbird;
	
	const int depth_limit = hummingbird.depth_limit - 1;
	
	for (int a = 0; a < problems.size(); a++) {
		
		auto [visual, good_checkmate_depth, include] = problems[a];
		if (!include)
			continue;
		
		cout << "------------" << endl;
		cout << "Puzzle " << a + 1 << endl;
		
		hummingbird.setup_visual(visual);
		hummingbird.game.display();
		
		const Color hummingbird_color = hummingbird.game.active_player;
		
		// Play game
		int move_count = 0;
		const int move_limit = 50;
		while (move_count < move_limit && !hummingbird.game.is_finished() && SimpleEndgameAI::is_sufficient_material(hummingbird.game)) {
			
			Move move;
			if (hummingbird.game.active_player == hummingbird_color) {
				move = hummingbird.find_best_move(depth_limit);
			}
			else {
				move = SimpleEndgameAI::find_best_move(hummingbird.game);
			}
			cout << Notation::move_to_string(move) << " ";
			hummingbird.game.apply(move);
			move_count++;
		}
		cout << endl;
		cout << endl;
		
		if (hummingbird.game.is_checkmate()) {
			if (move_count <= good_checkmate_depth) {
				cout << "Excellent!" << endl;
			}
			else {
				cout << "Hummingbird checkmated opponent in " << move_count << " but should have only taken " << good_checkmate_depth << ":" << endl;
			}
		}
		else if (hummingbird.game.is_stalemate()) {
			// Stalemate
			cout << "Hummingbird slipped into stalemate:" << endl;
		}
		else if (!SimpleEndgameAI::is_sufficient_material(hummingbird.game)) {
			// Hummingbird lost its pieces
			cout << "Hummingbird ran out of sufficient material:" << endl;
		}
		else {
			// Hummingbird was unable to checkmate in reasonable time
			cout << "Hummingbird took " << move_limit << " moves without checkmating opponent:" << endl;
		}
		
		hummingbird.game.display();
		cout << endl;
	}
}

} // namespace HummingbirdTester

#endif /* hummingbird_tester_h */

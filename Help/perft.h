//
//  perft.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#pragma once
#ifndef perft_h
#define perft_h

#include "fruit.h"
#include "game.h"
#include <cmath>

namespace Perft
{

inline uint64_t skipped_nodes = 0;
inline int max_depth;
inline Table<PerftEntry> table(10'000'000);

template<Variant V>
uint64_t perft(Game<V> &game, int depth)
{
	if (depth == max_depth) return 1;
	
	// Check for alternative win
	if constexpr (Variants::has_alternative_winning_condition(V)) {
		if (game.is_alternative_winning_condition_met(game.active_player))
			return 1;
		if (game.is_alternative_winning_condition_met(!game.active_player))
			return 1;
	}
	
	// Query the transposition table
	{
		PerftEntry *table_entry = table.get(game.hash);
		uint64_t answer;
		if (table_entry && (answer = table_entry->node_count[depth])) {
			skipped_nodes += answer;
			return answer;
		}
	}
	
	uint64_t count = 0;
	game.generate_quasilegal_moves();
	std::vector<Move> moves = game.quasilegal_moves;
	if (depth == max_depth - 1) {
		for (Move move : moves)
			if (game.attempt(move)) {
				count++;
				game.undo();
			}
	}
	else {
		for (Move move : moves) {
			if (game.attempt(move)) {
				count += perft(game, depth + 1);
				game.undo();
			}
		}
	}
	
	// Update the transposition table
	{
		PerftEntry *table_entry = table.get_pointer(game.hash);
		if (table_entry->does_exist()) {
			if (table_entry->key == game.hash) {
				// Simply update the entry
				table_entry->node_count[depth] = count;
			}
			else {
				// Consider whether to replace the entry
				table_entry->key = game.hash;
				std::fill(table_entry->node_count.begin(), table_entry->node_count.end(), 0);
				table_entry->node_count[depth] = count;
			}
		}
		else {
			// Create a new entry
			table_entry->key = game.hash;
			table_entry->exists = true;
			table_entry->node_count[depth] = count;
		}
	}
	
	return count;
}

template<Variant V>
std::map<std::string, uint64_t> divide(Game<V> &game, int depth)
{
	std::map<std::string, uint64_t> branches;
	if (depth == 0)
		return branches;
	
	max_depth = depth;
	skipped_nodes = 0;
	game.generate_quasilegal_moves();
	std::vector<Move> moves = game.quasilegal_moves;
	for (Move move : moves) {
		if (game.attempt(move)) {
			uint64_t count = perft(game, 1);
			branches[Notation::move_to_string(move)] = count;
			game.undo();
		}
	}
	return branches;
}

template<Variant V>
void speed_test()
{
	/*
	 
	 --- Depth 4 ---
	 Nodes: 4,085,603
	 Nodes per second: 10,755k
	 Total time: 0.38
	 
	 --- Depth 5 ---
	 Nodes: 193,690,690
	 Nodes per second: 16,732k
	 Total time: 11.58
	 
	 --- Depth 5 ---
	 Nodes: 193,690,690
	 Nodes per second: 14,682k
	 Total time: 13.19
	 
	 --- Depth 5 ---
	 Nodes: 193,690,690
	 Nodes per second: 33,424k
	 Skipped nodes: 119,191,767
	 Total time: 5.79
	 
	 --- Depth 5 ---
	 Nodes: 193,690,690
	 Nodes per second: 51,998k
	 Skipped nodes: 127,797,453
	 Total time: 3.72
	 
	 
	 --- Depth 6, Low Power Mode ---
	 Nodes: 8,031,647,685
	 Nodes per second: 57,347k
	 Skipped nodes: 5,934,571,451
	 Total time: 140.05
	 
	 --- Depth 6 ---
	 Nodes: 8,031,647,685
	 Nodes per second: 95,888k
	 Skipped nodes: 5,934,358,481
	 Total time: 83.76
	 
	 */
	
	cout << "Performing speed test..." << endl;
	
	const int depth = 5;
	
	Game<V> game;
	game.setup_fen("kiwi");
	game.display();
	
	fruit::Stopwatch stopwatch;
	stopwatch.start();
	
	std::map<std::string, uint64_t> result = divide(game, depth);
	
	double time_taken = stopwatch.check();
	uint64_t node_count = 0;
	for (auto element : result) {
		node_count += element.second;
	}
	
	cout << "Nodes: " << fruit::thousands_separated_by_commas(node_count) << endl;
	int nodes_per_second = (int)((double)node_count / time_taken);
	std::string kilonodes_per_second = fruit::thousands_separated_by_commas(nodes_per_second / 1'000) + "k";
	cout << "Nodes per second: " << kilonodes_per_second << endl;
	cout << "Skipped nodes: " << fruit::thousands_separated_by_commas(skipped_nodes) << endl;
	cout << "Total time: " << std::round(time_taken * 100) / 100 << endl;
}

bool advanced_analysis_fen(const std::string &fen, int depth);
void advanced_analysis_file(const std::string &file_name, int max_nodes);

std::map<std::string, uint64_t> stockfish_divide(const std::string &fen, int depth);

} // namespace Perft

#endif /* perft_h */

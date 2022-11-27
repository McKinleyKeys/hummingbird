//
//  perft.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#include "perft.h"
#include "notation.h"
#include "table.h"

namespace Perft
{

void reset()
{
	table.reset();
}

bool advanced_analysis_fen(const std::string &fen, int depth)
{
	cout << "Running advanced analysis on " << (fen == "" ? "default start position" : fen) << " with depth " << depth << endl;
	cout << endl;
	
	Game<CLASSIC> game;
	game.setup_fen(fen);
	
	for (int subdepth = depth; subdepth >= 0; subdepth--) {
		
		cout << "----------------" << endl;
		cout << endl;
		cout << "Running Perft on:" << endl;
		cout << game.debug_description() << endl;
		cout << game.fen() << endl;
		cout << endl;
		
		// Run Hummingbird
		reset();
		fruit::Stopwatch stopwatch;
		stopwatch.start();
		std::map<std::string, uint64_t> hummingbird_results = divide(game, subdepth);
		double time_taken = stopwatch.check();
		
		// Run stockfish
		std::map<std::string, uint64_t> stockfish_results = stockfish_divide(game.fen(), subdepth);
		
//		cout << "Hummingbird results:" << endl;
//		for (auto element : hummingbird_results) {
//			cout << element.first << ": " << element.second << endl;
//		}
		uint64_t node_count = 0;
		for (auto element : hummingbird_results)
			node_count += element.second;
		cout << "Total nodes: " << fruit::thousands_separated_by_commas(node_count) << endl;
		cout << endl;
//		cout << "Stockfish results:" << endl;
//		for (auto element : stockfish_results) {
//			cout << element.first << ": " << element.second << endl;
//		}
		uint64_t stockfish_count = 0;
		for (auto element : stockfish_results)
			stockfish_count += element.second;
		cout << "Total nodes: " << fruit::thousands_separated_by_commas(stockfish_count) << endl;
		cout << endl;
		
		// Merge the keys
		std::vector<std::string> all_keys;
		all_keys.reserve(hummingbird_results.size() + stockfish_results.size());
		for (auto element : hummingbird_results)
			all_keys.push_back(element.first);
		for (auto element : stockfish_results)
			all_keys.push_back(element.first);
		
		// Any move that produces a different count from the two engines
		Move different_move = 0;
		for (const std::string &key : all_keys) {
			
			if (hummingbird_results.contains(key) && stockfish_results.contains(key)) {
				if (hummingbird_results[key] != stockfish_results[key]) {
					// This move produces a different count
					different_move = Notation::parse_move(key, game);
				}
			}
			else {
				// One of the engines must think this move is possible while the other one does not
				cout << game.debug_description() << endl;
				cout << endl;
				cout << game.fen() << endl;
				cout << endl;
				if (!hummingbird_results.contains(key)) {
					cout << "Hummingbird does not realize that " << key << " is legal in the above position" << endl;
				}
				else {
					cout << "Hummingbird erroneously thinks " << key << " is legal in the above position" << endl;
				}
				return false;
			}
		}
		
		if (different_move) {
			// Apply this move and keep looping
			cout << "Applying move: " << Notation::move_to_string(different_move) << endl;
			cout << endl;
			game.apply(different_move);
		}
		else {
			if (subdepth == depth) {
				// The two engines agree completely
				cout << "Hummingbird performed perfectly" << endl;
				// Print the nodes per second styled nicely
				int nodes_per_second = int(double(node_count) / time_taken);
				const std::string kilonodes_per_second = fruit::thousands_separated_by_commas(nodes_per_second / 1'000) + "k";
				cout << "Nodes per second: " << kilonodes_per_second << endl;
				return true;
			}
			else {
				// There must have been a disagreement earlier, since this isn't the first iteration of the loop, but now the two engines are agreeing
				cout << "Hummingbird is flip-flopping more than beach-goers!" << endl;
				return false;
			}
		}
	}
	return true;
}

void advanced_analysis_file(const std::string &file_name, int max_nodes)
{
	// Read file contents
	const std::string file_url = "/Users/mckinley/Desktop/McKinley/Programming/Chess/" + file_name;
	const std::optional<std::string> optional_contents = fruit::slurp(file_url);
	if (!optional_contents.has_value()) {
		cout << "Could not load file " << fruit::debug_description(file_name) << endl;
		cout << "Aborting advanced Perft analysis" << endl;
		return;
	}
	const std::string contents = optional_contents.value();
	
	std::vector<std::string> lines = fruit::split(contents, '\n');
	for (int a = 0; a < lines.size(); a++) {
		lines[a] = fruit::trimming_whitespace(lines[a]);
	}
	
	cout << "Reading Perft problems from " << fruit::debug_description(file_name) << "..." << endl;
	cout << endl;
	
	bool all_correct = true;
	for (const std::string &line : lines) {
		if (!line.starts_with("//")) {
			
			if (line == "STOP") break;
			
			cout << "--------------------------------" << endl;
			cout << endl;
			
			// The line should be of the following form:
			// "FEN_STRING" (perft DEPTH = NODE_COUNT)
			std::string fen = "";
			int depth = 0;
			int node_count = 0;
			if (line.starts_with("\"")) {
				
				size_t end_quote = line.find('"', 1);
				if (end_quote != std::string::npos) {
					
					// The fen is in-between the quotes
					fen = line.substr(1, end_quote - 1);
					
					size_t open_parenthesis = line.find('(', end_quote + 1);
					if (open_parenthesis != std::string::npos) {
						
						size_t close_parenthesis = line.find(')', open_parenthesis + 1);
						if (close_parenthesis != std::string::npos) {
							
							// The depth and node count are between the parentheses
							const std::vector<std::string> components = fruit::split(line.substr(open_parenthesis + 1, close_parenthesis - open_parenthesis - 1), ' ', false);
							if (components.size() == 4 && components[0] == "perft" && components[2] == "=") {
								try {
									int _depth = std::stoi(components[1]);
									int _node_count = std::stoi(fruit::replacing_characters(components[3], ",", ""));
									if (_depth > 0 && _node_count > 0) {
										depth = _depth;
										node_count = _node_count;
									}
								}
								catch (...) {
									cout << "Invalid format: " << fruit::debug_description(line) << endl;
								}
							}
						}
					}
				}
			}
			
			if (fen.size() && depth && node_count) {
				
				if (max_nodes && node_count > max_nodes) {
					cout << "Skipping Perft problem: " << line << endl;
				}
				else {
					// Run Perft on the problem
					bool result = advanced_analysis_fen(fen, depth);
					if (!result) {
						// Terminate the search
						all_correct = false;
						cout << endl;
						break;
					}
				}
			}
			else {
				cout << "Incountered incorrectly formatted Perft problem: " << line << endl;
			}
			
			cout << endl;
		}
	}
	
	if (all_correct) {
		cout << "Finished all Perft problems successfully" << endl;
	}
	else {
		cout << "Aborting Perft analysis" << endl;
	}
}


// MARK: - Stockfish

std::string execute_stockfish(std::vector<std::string> &supplemental_commands)
{
	supplemental_commands.push_back("quit");
	return fruit::execute("/opt/homebrew/bin/stockfish", supplemental_commands);
}

std::map<std::string, uint64_t> stockfish_divide(const std::string &fen, int depth)
{
	std::map<std::string, uint64_t> result;
	if (depth == 0)
		return result;
	
	std::vector<std::string> commands;
	if (fen != "")
		commands.push_back("position fen " + fen + " moves");
	commands.push_back("go perft " + std::to_string(depth));
	
	const std::string output = execute_stockfish(commands);
	for (const std::string &line : fruit::split(output, '\n')) {
		const std::vector<std::string> line_split = fruit::split(line, ':');
		if (line_split.size() == 2) {
			const std::string first = fruit::trimming_whitespace(line_split[0]);
			const std::string second = fruit::trimming_whitespace(line_split[1]);
			if (first.size() == 4 || first.size() == 5) {
				// Try parsing `second`
				try {
					uint64_t count = std::stoull(second);
					result[first] = count;
				}
				catch (...) {
					// Ignore line
				}
			}
		}
	}
	
	return result;
}

} // namespace Perft

//
//  uci.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/18/22.
//

#include "uci.h"
#include "game.h"
#include "hummingbird.h"
#include "perft.h"
#include "definitions.h"

// Hummingbird conforms to the UCI protocol as defined in https://backscattering.de/chess/uci/.

namespace UCI
{

Variant current_variant = CLASSIC;

bool is_quit = false;
fruit::DispatchQueue background_queue("com.mckinleykeys.chaos-chess.hummingbird.uci-background-queue");

template<Variant V>
class Session
{
	std::istringstream line_stream;
	
	Hummingbird<V> hummingbird;
	bool should_end_session;
	
public:
	
	Session() : should_end_session(false)
	{
		hummingbird.default_setup();
	}
	
private:
	
	bool has_next_token()
	{
		return !line_stream.eof();
	}
	std::string next_token()
	{
		if (!has_next_token())
			return "";
		std::string token;
		line_stream >> token;
		token = fruit::to_lowercase(token);
		return token;
	}
	
	void setoption()
	{
		std::string token;
		
		// Consume "name"
		token = next_token();
		if (token != "name")
			return;
		
		token = next_token();
		if (token == "FiftyMoveRule") {
			// Consume "value"
			token = next_token();
			if (token != "value")
				return;
			// Get the value
			token = next_token();
			if (token == "true")
				hummingbird.game.fifty_move_rule_enabled = true;
			if (token == "false")
				hummingbird.game.fifty_move_rule_enabled = false;
		}
	}
	void position()
	{
		std::string token;
		token = next_token();
		
		if (token == "startpos") {
			hummingbird.default_setup();
			// Consume the "moves" token, if it exists
			// TODO: ONLY CONSUME IF IT EXISTS
			token = next_token();
		}
		else if (token == "fen") {
			std::string fen = "";
			while (has_next_token()) {
				token = next_token();
				if (token == "moves") break;
				fen += token + " ";
			}
			hummingbird.setup_fen(fen);
		}
		else {
			// Load custom named position
			const auto result = Notation::named_positions.find(token);
			if (result != Notation::named_positions.end())
				hummingbird.setup_fen(result->second);
			// Consume the "moves" token, if it exists
			token = next_token();
		}
		
		// Read moves
		while (has_next_token()) {
			token = next_token();
			Move move = Notation::parse_move(token, hummingbird.game);
			if (move)
				hummingbird.apply(move);
		}
	}
	void go()
	{
		std::string token;
		
		bool perft = false;
		int depth_limit = 0;
		uint64_t node_limit = 0;
		double move_time_limit = 0;
		bool infinite = false;
		
		while (has_next_token()) {
			
			token = next_token();
			
			if (token == "depth") {
				token = next_token();
				try {
					depth_limit = std::stoi(token);
				}
				catch (...) {
					cout << "Invalid depth limit specified" << endl;
				}
			}
			else if (token == "nodes") {
				token = next_token();
				try {
					node_limit = std::stoull(token);
				}
				catch (...) {
					cout << "Invalid node limit specified" << endl;
				}
			}
			else if (token == "movetime") {
				token = next_token();
				try {
					uint64_t milliseconds = std::stoull(token);
					move_time_limit = (double)milliseconds / 1000;
				}
				catch(...) {
					cout << "Invalid move time limit specified" << endl;
				}
			}
			else if (token == "perft") {
				perft = true;
				token = next_token();
				try {
					depth_limit = std::stoi(token);
				}
				catch (...) {
					cout << "Invalid depth limit specified" << endl;
				}
			}
			else if (token == "infinite")
				infinite = true;
		}
		
		if (perft) {
			if (depth_limit) {
				background_queue.async([depth_limit, this]() {
					Perft::table.reset();
					auto results = Perft::divide(hummingbird.game, depth_limit);
					cout << endl;
					for (auto entry : results)
						cout << entry.first << ": " << entry.second << endl;
					cout << endl;
				});
			}
			else {
				cout << "No depth limit specified for 'perft' command" << endl;
				cout << endl;
			}
		}
		else {
			if (move_time_limit > 0) {
				background_queue.async([depth_limit, move_time_limit, this]() {
					Move move = hummingbird.find_best_move(depth_limit, move_time_limit);
					cout << "bestmove " << Notation::move_to_string(move) << endl;
					cout << endl;
				});
			}
			else {
				background_queue.async([depth_limit, this]() {
					Move move = hummingbird.find_best_move(depth_limit);
					cout << "bestmove " << Notation::move_to_string(move) << endl;
					cout << endl;
				});
			}
		}
	}
	void ucinewgame()
	{
		// Nothing to do
	}
	void display()
	{
		std::string token;
		
		if (has_next_token()) {
			
			token = next_token();
			
			if (token == "variant") {
				cout << "<" << Notation::variant_to_string(V) << ">" << endl;
				return;
			}
		}
		
		cout << hummingbird.game.visual() << endl;
		cout << endl;
	}
	void variant()
	{
		std::string token;
		
		if (!has_next_token())
			return;
		
		// Some variants' names are multiple words. To be recognized, they must be surrounded by carets, like "<Compulsion & Temptation>".
		std::string name;
		
		int unescaped_carets = 0;
		do {
			token = next_token();
			if (token.starts_with("<")) {
				unescaped_carets++;
				token = token.substr(1);
			}
			if (token.ends_with(">")) {
				unescaped_carets--;
				token.pop_back();
			}
			name += token;
		}
		while (unescaped_carets > 0 && has_next_token());
		
		Variant new_variant = Notation::parse_variant(name);
		if (new_variant != UNRECOGNIZED_VARIANT) {
			current_variant = new_variant;
			cout << "Changed variant to <" << Notation::variant_to_string(current_variant) << ">" << endl;
			should_end_session = true;
		}
	}
	
	void execute_command(const std::string line)
	{
		std::string token;
		line_stream = std::istringstream(line);
		
		while (has_next_token()) {
			
			// Read the next token
			token = next_token();
			
			// MARK: - Standard UCI Commands
			
			if (token == "uci") {
				cout << "id name " << "Hummingbird" << endl;
				cout << "id author " << "McKinley Keys" << endl;
				// Indicate that Hummingbird uses its own opening book by default
				cout << "option name OwnBook type check default true" << endl;
				// Indicate that Hummingbird uses the fifty move rule by default
				cout << "option name FiftyMoveRule type check default true" << endl;
				cout << "uciok" << endl;
			}
			else if (token == "setoption")
				setoption();
			else if (token == "position")
				position();
			else if (token == "go")
				go();
			else if (token == "stop")
				hummingbird.stop_immediately();
			else if (token == "ucinewgame")
				ucinewgame();
			else if (token == "isready")
				cout << "readyok" << endl;
			else if (token == "quit" || token == "q") {
				is_quit = true;
				should_end_session = true;
				break;
			}
			
			// MARK: - Custom Commands
			
			else if (token == "d")
				display();
			else if (token == "variant")
				variant();
			
			else {
				// We encountered an unregonized UCI command. As per the specification, we should ignore the token and continue parsing the line.
				continue;
			}
			
			// A command was executed. Ignore the rest of the line.
			break;
		}
	}
	
public:
	void run()
	{
		std::string line;
		while (!should_end_session && !is_quit) {
			if (!getline(cin, line))
				break;
			execute_command(line);
		}
	}
};

// Explicitly instantiate the `Session` class with each variant
//template class Session<CLASSIC>;
//template class Session<ENERGY_CELLS>;
//template class Session<EXPLODING_KNIGHTS>;
//template class Session<COMPULSION>;
//template class Session<COMPULSION_AND_BACKSTABBING>;
//template class Session<PETRIFICATION>;
//template class Session<FORCED_CHECK>;
//template class Session<LOSER>;
//template class Session<KING_OF_THE_HILL>;
//template class Session<KING_OF_THE_HILL_AND_COMPULSION>;
//template class Session<THREE_CHECK>;
//template class Session<FOG_OF_WAR>;

#define INSTANTIATE_SESSION(VARIANT) \
	template class Session<VARIANT>;

FOR_EACH_VARIANT(INSTANTIATE_SESSION)


void run()
{
	cout << "Running Hummingbird in UCI mode" << endl;
	
	while (!is_quit) {
		switch (current_variant) {
			#define SWITCH_CASE_FOR_VARIANT(V) \
				case V: { \
					Session<V> session; \
					session.run(); \
					break; \
				}
			FOR_EACH_VARIANT(SWITCH_CASE_FOR_VARIANT)
			default:
				goto cleanup;
		}
	}
	cleanup:;
}

} // namespace UCI

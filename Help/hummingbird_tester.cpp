//
//  hummingbird_tester.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/12/22.
//

#include "hummingbird_tester.h"
#include "game.h"
#include "hummingbird.h"
#include "uci.h"

namespace HummingbirdTester
{

void compare_elo(int argc, char **argv)
{
	// Cute Chess requires all engines to be compiled executables, so we must supply it the path of the Hummingbird executable, which happens to be the first string in `argv`. However, that executable is programmed to enter this `compare_elo()` function, not `UCI::run()`. We fix this by telling Cute Chess to pass a special command line argument to the Hummingbird executable that will tell us to go to `UCI::run()`.
	
	if (argc == 0) {
		cout << "Error: Hummingbird was executed with 0 command line arguments" << endl;
		return;
	}
	const std::string path_to_new_hummingbird_executable(argv[0]);
	if (argc >= 2) {
		std::string second_argument(argv[1]);
		if (second_argument == "uci")
			return UCI::run();
	}
	
	
	// MARK: - Settings
	
	const std::string old_hummingbird = "Hummingbird v0.1";
	const std::string path_to_old_hummingbird_executable = "/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines/" + old_hummingbird + "/Products/usr/local/bin/hummingbird";
	
	const std::string new_hummingbird_name = "Hummingbird Prime";
	const std::string old_hummingbird_name = "Hummingbird";
	
	const std::string tournament_title = "Hummingbird Evolution 1";
	// 2 minutes * game_count * time_per_move
//	const int game_count = 32;
//	const double time_per_move = 1;
	const int game_count = 8;
	const double time_per_move = 0.1;
	const bool debug = true;
	const int simultaneous_game_count = 1;
	const std::string pgn_output_file = "/Users/mckinley/Desktop/" + tournament_title + ".pgn";
	
	
	// MARK: - Build Command
	
	std::vector<std::string> tokens;
	tokens.push_back("/usr/local/bin/cutechess");
	
	// Add the new Hummingbird
	tokens.push_back("-engine");
	tokens.push_back("name='Hummingbird Prime'");
	tokens.push_back("cmd='" + path_to_new_hummingbird_executable + "'");
	tokens.push_back("arg='uci'");
	tokens.push_back("option.FiftyMoveRule=false");
	
	// Add the old Hummingbird
	tokens.push_back("-engine");
	tokens.push_back("name='Hummingbird'");
	tokens.push_back("cmd='" + path_to_old_hummingbird_executable + "'");
	tokens.push_back("arg='uci'");
	tokens.push_back("option.FiftyMoveRule=false");
	
	tokens.push_back("-event '" + tournament_title + "'");
	// The number of games per encounter
	tokens.push_back("-games " + std::to_string(game_count));
	// Print Elo estimates after every game
	tokens.push_back("-ratinginterval 1");
	// Allow multiple games to be played simultaneously
	if (simultaneous_game_count > 1)
		tokens.push_back("-concurrency " + std::to_string(simultaneous_game_count));
	
	tokens.push_back("-each");
	tokens.push_back("proto=uci");
	// Set the time per move, in seconds
	tokens.push_back("st=" + std::to_string(time_per_move));
	// Set the time margin, in milliseconds (the Cute Chess documentation says that this option should be in seconds, but it is incorrect).
	tokens.push_back("timemargin=20000");
	// Enable pondering
	tokens.push_back("ponder");
	// Set the max number of full moves until the game is adjudicated as a draw
//	tokens.push_back("-maxmoves 80");
	// Configure draw conditions (this doesn't seem to work at all)
//	tokens.push_back("-draw");
//	tokens.push_back("movenumber=1");		// Set the fifty move rule to be enabled at the start of the game
//	tokens.push_back("movecount=75");		// Set the fifty move rule to end the game if 75 plies have passed without irreversible moves
//	tokens.push_back("score=100000000");	// Set the fifty move rule to be enabled no matter how large of the score margin in centipawns
	// Send the PGN output to a file
	tokens.push_back("-pgnout '" + pgn_output_file + "'");
	if (debug)
		tokens.push_back("-debug");
	
	const std::string command = fruit::join(tokens, " ");
	
	cout << "Running..." << endl;
	cout << command << endl;
	cout << endl;
	cout << "------------------------------------------------" << endl;
	cout << endl;
	
	fruit::Stopwatch stopwatch;
	stopwatch.start();
	
	int completed_game_count = 0;
	std::map<std::string, std::pair<std::string, std::string>> engine_elos;
	
	// Execute the command and monitor its output
	const std::string result = fruit::execute(command, [&](const std::string line) {
		
		const std::vector<std::string> tokens = fruit::tokenize(line);
		
		if (line.starts_with("Finished game")) {
			
			completed_game_count++;
			double progress = (double)completed_game_count / game_count;
			progress *= 100;
			int percent = (int)std::round(progress);
			cout << "Finished Game " << completed_game_count << " / " << game_count << " (" << percent << "%)" << endl;
			engine_elos.clear();
			
			// Output game result
			std::string result = "";
			for (int index = 0; index < tokens.size(); index++)
				if (tokens[index].starts_with("{")) {
					result = tokens[index - 1];
					break;
				}
			if (result != "") {
				if (result == "1/2-1/2")
					cout << "(Draw)" << endl;
				if (result == "1-0")
					cout << (completed_game_count % 2 == 1 ? "(Win)" : "(Loss)") << endl;
				if (result == "0-1")
					cout << (completed_game_count % 2 == 1 ? "(Loss)" : "(Win)") << endl;
			}
		}
		
		// Check whether this is an Elo output line
		if (line.starts_with("Elo difference")) {
			
			std::string elo_difference = tokens[2];
			std::string elo_error = tokens[4];
			
			// Remove the comma at the end of `elo_error`
			elo_error = elo_error.substr(0, elo_error.size() - 1);
			
			// Cut off the decimal points off `elo_difference` and `elo_error`
			if (elo_difference != "inf" && elo_difference != "-inf" && elo_difference != "nan") {
				double value = std::round(std::stod(elo_difference));
				elo_difference = std::to_string((int)value);
			}
			if (elo_error != "inf" && elo_error != "-inf" && elo_error != "nan") {
				double value = std::round(std::stod(elo_error));
				elo_error = std::to_string((int)value);
			}
			
			cout << "Elo difference:\t\t" << elo_difference << " +/- " << elo_error << endl;
			cout << endl;
			cout << endl;
		}
	});
	
	double time_taken = stopwatch.check();
	
	cout << endl;
	cout << endl;
	cout << "Finished " << tournament_title << "!" << endl;
	cout << endl;
	cout << endl;
	cout << "------------------------------------------------" << endl;
	cout << endl;
	cout << endl;
	cout << result << endl;
	cout << endl;
	cout << endl;
	cout << "Time taken: " << time_taken << " seconds " << endl;
	cout << endl;
}
void calculate_elo(int argc, char **argv)
{
	// Cute Chess requires all engines to be compiled executables, so we must supply it the path of the Hummingbird executable, which happens to be the first string in `argv`. However, that executable is programmed to enter this `calculate_elo()` function, not `UCI::run()`. We fix this by telling Cute Chess to pass a special command line argument to the Hummingbird executable that will tell us to go to `UCI::run()`.
	
	if (argc == 0) {
		cout << "Error: Hummingbird was executed with 0 command line arguments" << endl;
		return;
	}
	const std::string path_to_hummingbird_executable(argv[0]);
	if (argc >= 2) {
		std::string second_argument(argv[1]);
		if (second_argument == "uci")
			return UCI::run();
	}
	
	// **Engines**
	// (Elo ratings are according to the CCRL 40/15 rating list: http://www.computerchess.org.uk/ccrl/4040/)
	// Stockfish: https://github.com/official-stockfish/Stockfish (~3500 Elo)
	// Rustic: https://github.com/mvanthoor/rustic (unknown Elo)
	// Snowy: https://github.com/JasonCreighton/snowy (~2000 Elo)
	// Pulse: https://github.com/fluxroot/pulse (~1600 Elo)
	// Dionysus: https://github.com/patterson-tom/Dionysus (unknown Elo)
	
	// Current estimate: ~1490 +/- 215
	
	std::map<std::string, std::string> engine_commands =
	{
		{ "Hummingbird", "cmd='" + path_to_hummingbird_executable + "' arg='uci'" },
		{ "Stockfish", "cmd='/opt/homebrew/bin/stockfish'" },
		{ "Rustic", "cmd='/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines/rustic'" },
		{ "Snowy", "cmd='/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines/snowy'" },
		{ "Pulse", "cmd='/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines/pulse' dir='/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines'" },
		{ "Dionysus", "cmd='/Users/mckinley/Desktop/McKinley/Programming/Chess/Engines/dionysus'" },
	};
	std::map<std::string, int> known_elos =
	{
		{ "Stockfish", 3500 },
		{ "Snowy", 2000 },
		{ "Pulse", 1600 },
	};
	
	
	// MARK: - Settings
	
	const std::string tournament_title = "Battlechess 1";
	const double time_per_move = 2.5;
	const int games_per_encounter = 8;
	const bool round_robin = true;
	const int simultaneous_game_count = 1;
	const std::string pgn_output_file = "/Users/mckinley/Desktop/" + tournament_title + ".pgn";
	
	std::vector<std::string> tournament_participants =
	{
		"Hummingbird",
//		"Stockfish",
		"Rustic",
		"Snowy",
		"Pulse",
		"Dionysus",
	};
	
	
	std::vector<std::string> tokens;
	tokens.push_back("/usr/local/bin/cutechess");
	
	// Add each engine
	for (std::string engine_name : tournament_participants) {
		const std::string command = engine_commands[engine_name];
		tokens.push_back("-engine");
		tokens.push_back(command);
		// Assign the engine's name to be `engine_name` so that we can recognize it easily in the output logs
		tokens.push_back("name='" + engine_name + "'");
	}
	
	tokens.push_back("-event '" + tournament_title + "'");
	// The number of games per encounter
	tokens.push_back("-games " + std::to_string(games_per_encounter));
//	tokens.push_back("-rounds 10");
	// Print Elo estimates after every game
	tokens.push_back("-ratinginterval 1");
	if (round_robin)
		tokens.push_back("-tournament round-robin");
	else
		tokens.push_back("-tournament gauntlet");
	// Allow multiple games to be played simultaneously
	if (simultaneous_game_count > 1)
		tokens.push_back("-concurrency " + std::to_string(simultaneous_game_count));
	
	tokens.push_back("-each");
	tokens.push_back("proto=uci");
	// Set the time per move, in seconds
	tokens.push_back("st=" + std::to_string(time_per_move));
	// Set the time margin, in milliseconds (the Cute Chess documentation says that this option should be in seconds, but it is incorrect). Engines don't get to know what the time margin is. We make this margin very high because some of the engines (not Hummingbird) are poor at submitting their best move on time. It's also possible that some background task on the computer slows down an engine for a second or two.
	tokens.push_back("timemargin=10000");
	// Enable pondering
	tokens.push_back("ponder");
	// Send the PGN output to a file
	tokens.push_back("-pgnout '" + pgn_output_file + "'");
	
	const int engine_count = (int)tournament_participants.size();
	const int game_count = games_per_encounter * (round_robin ? (engine_count * (engine_count - 1) / 2) : (engine_count - 1));
	
	const std::string command = fruit::join(tokens, " ");
	
	cout << "Running..." << endl;
	cout << command << endl;
	cout << endl;
	cout << "------------------------------------------------" << endl;
	cout << endl;
	
	fruit::Stopwatch stopwatch;
	stopwatch.start();
	
	int completed_game_count = 0;
	std::map<std::string, std::pair<std::string, std::string>> engine_elos;
	
	// Execute the command and monitor its output
	const std::string result = fruit::execute(command, [&](const std::string line) {
		
		const std::vector<std::string> tokens = fruit::split(line, ' ');
		
		if (line.starts_with("Finished game")) {
			completed_game_count++;
			double progress = (double)completed_game_count / game_count;
			progress *= 100;
			int percent = (int)std::round(progress);
			cout << "Finished Game " << completed_game_count << " / " << game_count << " (" << percent << "%)" << endl;
			engine_elos.clear();
		}
		
		// Check whether this is an Elo output line
		if (tokens.size() >= 4 && fruit::contains(tournament_participants, tokens[1])) {
			
			{
				const std::string engine_name = tokens[1];
				const std::string elo = tokens[2];
				const std::string elo_error = tokens[3];
				engine_elos[engine_name] = { elo, elo_error };
			}
			
			// Check whether we've received the data for all engines
			if (engine_elos.size() == engine_count) {
				
				// Try to find an anchor
				
				/// The real-world Elo to which a 0 Cute Chess Elo corresponds
				int elo_at_zero = 0;
				std::string anchor_engine_name = "";
				for (auto element : known_elos) {
					
					if (!fruit::contains(tournament_participants, element.first))
						continue;
					
					/// The Elo that Cute Chess has estimated
					const std::string estimated_elo = engine_elos[element.first].first;
					/// The known Elo
					const int known_elo = element.second;
					
					if (estimated_elo != "inf" && estimated_elo != "-inf" && estimated_elo != "nan") {
						// `elo` must be a number
						elo_at_zero = known_elo - std::stoi(estimated_elo);
						anchor_engine_name = element.first;
						break;
					}
				}
				
				if (anchor_engine_name != "")
					cout << "Using " << anchor_engine_name << " (" << known_elos[anchor_engine_name] << ") as an Elo anchor" << endl;
				cout << endl;
				
				// Print out all Elos
				for (std::string participant : tournament_participants) {
					
					// Name
					cout << " - " << participant;
					
					int spaces_after_name = 24;
					int spaces_after_elo = 8;
					
					// Elo
					const std::string elo = engine_elos[participant].first;
					std::string elo_string;
					if (elo == "inf" || elo == "-inf" || elo == "nan")
						elo_string = elo;
					else
						elo_string = std::to_string(std::stoi(elo) + elo_at_zero);
					
					// Gap after name
					if (elo_string.starts_with("-")) {
						spaces_after_name--;
						spaces_after_elo++;
					}
					cout << fruit::repeating_string(" ", spaces_after_name - (int)participant.size());
					
					cout << elo_string;
					
					// Gap after Elo
					cout << fruit::repeating_string(" ", spaces_after_elo - (int)elo_string.size());
					
					// Elo error
					cout << "+/- " << engine_elos[participant].second << endl;
				}
				
				cout << endl;
				cout << endl;
			}
		}
	});
	
	double time_taken = stopwatch.check();
	
	cout << endl;
	cout << endl;
	cout << "Finished " << tournament_title << "!" << endl;
	cout << endl;
	cout << endl;
	cout << "------------------------------------------------" << endl;
	cout << endl;
	cout << endl;
	cout << result << endl;
	cout << endl;
	cout << endl;
	cout << "Time taken: " << time_taken << " seconds " << endl;
	cout << endl;
}

} // namespace HummingbirdTester

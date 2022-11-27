//
//  main.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 12/30/21.
//

#include "fruit.h"
#include "bitboard.h"
#include "table.h"
#include "magic.h"
#include "perft.h"
#include "uci.h"
#include "hummingbird.h"
#include "hummingbird_tester.h"
#include "opening_book.h"
#include "notation.h"

void play_game();
void parse_args(int argc, char **argv);

constexpr Variant V = CLASSIC;

int main(int argc, char **argv)
{
	fruit::init();
	Bitboards::init();
	Zobrist::init();
	Magic::init();
	parse_args(argc, argv);
	
//	Hummingbird<V> hummingbird;
//	hummingbird.load_default_opening_book();
//	hummingbird.opening_book.sanity_check();
//	cout << hummingbird.opening_book.loaded << endl;
	
	Perft::speed_test<CLASSIC>();
//	Perft::advanced_analysis_file("Perft Problems.txt", 200'000'000);
	
//	HummingbirdTester::speed_test<CLASSIC>();
//	HummingbirdTester::rigorous_test<V>();
//	HummingbirdTester::puzzle_test<V>();
//	HummingbirdTester::endgame_test<V>();
//	HummingbirdTester::calculate_elo(argc, argv);
//	HummingbirdTester::compare_elo(argc, argv);
	
//	UCI::run();
	
//	cout << "Search path is " << OpeningBooks::search_path << endl;
	
	
	
	
//	Game<V> &game = hummingbird.game;
//	game.default_setup();
//	game.apply(Notation::parse_move<V>("e2e4", game));
//	game.apply(Notation::parse_move<V>("c7c5", game));
//	game.apply(Notation::parse_move<V>("d2d4", game));
//	Move move = hummingbird.find_best_move(10);
//	cout << Notation::move_to_string(move) << endl;
	
//	Game<V> &game = hummingbird.game;
//	game.default_setup();
////	game.apply(Notation::parse_move("e2e4", game));
////	game.setup_visual(R"(
////		r n b . k b n r
//// 		p p p q p p p p
//// 		. . . . . . . .
//// 		. . . . . . N .
//// 		. . . . . . . .
//// 		. . . . . . . .
//// 		. . . . P P P .
////		. . . Q B K . .
//// 		w - -
//// 	)");
//	game.display();
//	game.apply(game.parse_algebraic("Nf3"));
//	game.apply(game.parse_algebraic("c6"));
//	game.apply(game.parse_algebraic("Ng5"));
//	game.apply(game.parse_algebraic("c5"));
//	game.apply(game.parse_algebraic("Nxf7"));
//	game.display();
//	game.generate_quasilegal_moves();
//	for (Move move : game.quasilegal_moves)
//		cout << Notation::move_to_string(move) << endl;
	
//	Move move = hummingbird.find_best_move(6);
//	cout << Notation::move_to_string(move) << endl;
//	game.setup_visual(R"(
//		r n b q k b n r
//		p p p p p p p p
//		. . . . . . . .
//		. . . . . . N .
//		. . . . . . . .
//		. . . . . . . .
//		P P P P P P P P
//		R N B Q K B . R
//		w - -
//	)");
//	game.display();
//	Move move = game.parse_algebraic("Nf7");
//	game.apply(move);
//	game.display();
//	game.sanity_check();
//	for (Move move : game.legal_moves()) {
//		cout << game.universal_notation(move) << endl;
//	}
	
//	play_game(game);
}

void parse_args(int argc, char **argv)
{
	// Boost has a good argument parser library: https://www.boost.org/doc/libs/1_49_0/doc/html/program_options/tutorial.html#id2499896
	for (int i = 1; i < argc; i++) {
		const std::string arg(argv[i]);
		if (arg == "--opening-book-dir") {
			i++;
			OpeningBooks::search_path = std::filesystem::path(std::string(argv[i]));
		}
	}
}

void play_game()
{
	bool player_is_hummingbird[2] = {true, false};
	
	Hummingbird<V> hummingbird;
	hummingbird.default_setup();
	
	while (!hummingbird.game.is_finished()) {
		
		cout << hummingbird.game.debug_description() << endl;
		cout << endl;
		
		if (player_is_hummingbird[hummingbird.game.active_player]) {
			// This player is a hummingbird
			cout << "Analyzing..." << endl;
			Move move = hummingbird.find_best_move(hummingbird.depth_limit);
			if (move) {
				cout << Notation::move_to_string(move) << endl;
				hummingbird.game.apply(move);
			}
			else {
				cout << "Hummingbird returned null" << endl;
				break;
			}
		}
		else {
			// This player is a human
			cout << "Please make a move." << endl;
			std::vector<Move> legal_moves = hummingbird.game.legal_moves();
			while (true) {
				std::string input;
				cin >> input;
				if (input != "") {
					Move move = Notation::parse_move(input, hummingbird.game);
					if (fruit::contains(legal_moves, move)) {
						hummingbird.game.apply(move);
						break;
					}
				}
				cout << "Invalid input. Please enter again." << endl;
			}
		}
		cout << endl;
	}
	if (hummingbird.game.is_checkmate())
		cout << "Checkmate!" << endl;
	else
		cout << "Stalemate" << endl;
	cout << endl;
}

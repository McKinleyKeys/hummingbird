//
//  table.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/11/22.
//

#include "table.h"
#include <random>

// Explicitly instantiate the `Table` class with each entry type. This needs to be done for every version of `Table<E>` to be accessible to other files.
template class Table<PerftEntry>;
template class Table<HummingbirdEntry>;


namespace Zobrist
{

HashKey keys[64][2][PIECE_COUNT];
HashKey active_player_key;
HashKey kingside_castling_keys[2];
HashKey queenside_castling_keys[2];
HashKey enpassant_keys[8];

constexpr int KEY_COUNT = 1024;
HashKey random_keys[KEY_COUNT];
int random_key_index;
HashKey next_random_key()
{
	if (random_key_index >= KEY_COUNT)
		fruit::fatal_error("Random numbers exhausted");
	return random_keys[random_key_index++];
}
void load_random_keys()
{
	std::mt19937_64 random_engine(26);
	for (int a = 0; a < KEY_COUNT; a++) {
		random_keys[a] = fruit::next_random<HashKey>(random_engine);
	}
	random_key_index = 0;
}

void init()
{
	load_random_keys();
	
	// `keys`
	for (int square = 0; square < 64; square++)
		for (int player = WHITE; player <= BLACK; player++)
			for (Piece piece = PAWN; piece <= KING; piece++)
				keys[square][player][piece] = next_random_key();
	
	// `kingside_castling_keys`
	for (int player = WHITE; player <= BLACK; player++)
		kingside_castling_keys[player] = next_random_key();
	
	// `queenside_castling_keys`
	for (int player = WHITE; player <= BLACK; player++)
		queenside_castling_keys[player] = next_random_key();
	
	// `active_player_key`
	active_player_key = next_random_key();
	
	// `enpassant_keys`
	for (int file = 0; file < 8; file++)
		enpassant_keys[file] = next_random_key();
}

} // namespace Zobrist

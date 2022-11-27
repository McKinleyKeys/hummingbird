//
//  table.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/11/22.
//

#pragma once
#ifndef table_h
#define table_h

#include "fruit.h"
#include "definitions.h"

template<class E>
class Table
{
private:
	std::vector<E> entries;
	int size;
	
public:
	Table(int _size) : size(_size), entries(_size)
	{}
	
	inline E* get(HashKey key)
	{
		E *entry = &entries[key % size];
		if (entry->does_exist() && entry->key == key)
			return entry;
		// TODO: DETERMINE WHETHER WE SHOULD RETURN NULL OR NULLPTR
		return NULL;
	}
	inline E* get_pointer(HashKey key)
	{
		return &entries[key % size];
	}
	inline void put(E entry)
	{
		entries[entry.key % size] = entry;
	}
	
	inline void reset()
	{
		std::fill(entries.begin(), entries.end(), E());
	}
};


struct PerftEntry
{
	HashKey key;
	bool exists;
	std::vector<uint64_t> node_count;
	
	PerftEntry(HashKey _key) : key(_key), exists(false), node_count(10) {}
	PerftEntry() : PerftEntry(0) {}
	
	inline bool does_exist()
	{
		return exists;
	}
};
struct HummingbirdEntry
{
	enum Precision : int
	{
		NONE = 0, EXACT = 1, LOWER_BOUND = 2, UPPER_BOUND = 3
	};
	
	HashKey key;
	Precision precision;
	int score;
	int remaining_depth;
	Move best_move;
	
	HummingbirdEntry(HashKey _key) : key(_key), precision(NONE), score(0), remaining_depth(0), best_move(NULL_MOVE) {}
	HummingbirdEntry() : HummingbirdEntry(0) {}
	
	inline bool does_exist()
	{
		return precision;
	}
};


namespace Zobrist
{

/// Usage: `keys[square][player][piece]`. The values at `keys[square][player][EMPTY]` are `0`.
extern HashKey keys[64][2][PIECE_COUNT];
extern HashKey active_player_key;
/// Usage: `kingside_castling_keys[player]`.
extern HashKey kingside_castling_keys[2];
/// Usage: `queenside_castling_keys[player]`.
extern HashKey queenside_castling_keys[2];
/// Usage: `enpassant_keys[file]`.
extern HashKey enpassant_keys[8];

void init();

} // namespace Zobrist

#endif /* table_h */

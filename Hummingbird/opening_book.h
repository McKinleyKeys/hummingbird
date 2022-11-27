//
//  opening_book.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/23/22.
//

#pragma once
#ifndef opening_book_h
#define opening_book_h

#include "fruit.h"
#include "game.h"
#include "definitions.h"
#include <random>

namespace OpeningBooks
{

extern std::filesystem::path search_path;

std::string book_url(const std::string &book_name);
std::string default_book_name_for_variant(Variant v);

} // namespace OpeningBooks


struct OpeningBook
{
	struct VisualEntry
	{
		struct Option
		{
			std::string notation;
			bool is_universal;
			int probability;
			Option(const std::string &_notation, bool _is_universal, int _probability);
		};
		
		std::string visual;
		std::vector<Option> options;
		
		template<typename E>
		Move random_option(const AbstractGame &game, E &random_engine) const;
		
		VisualEntry(const std::string &string);
		
		bool does_match_visual(const std::string &other_visual) const;
	};
	
	bool loaded;
	std::vector<VisualEntry> visual_entries;
	
	OpeningBook();
	OpeningBook(const std::string &book_name);
	void load(const std::string &book_name);
	
	void sanity_check() const;
	
	Move random_move(const AbstractGame &game) const;
	
private:
	mutable std::mt19937 random_engine;
};

#endif /* opening_book_h */

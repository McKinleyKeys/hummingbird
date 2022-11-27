//
//  opening_book.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/23/22.
//

#include "opening_book.h"
#include <ctime>

namespace OpeningBooks
{

std::filesystem::path search_path(".");

std::string book_url(const std::string &book_name)
{
	std::filesystem::path file_url = search_path / book_name;
	file_url.replace_extension("flexbook");
	return file_url.string();
}
std::string default_book_name_for_variant(Variant v)
{
	return fruit::capitalize_first_word(Notation::variant_to_universal(v));
}

} // namespace OpeningBooks


// MARK: - Opening Book

OpeningBook::OpeningBook() : loaded(false), visual_entries(), random_engine((unsigned int)std::time(nullptr))
{}

OpeningBook::OpeningBook(const std::string &book_name) : OpeningBook()
{
	load(book_name);
}


void OpeningBook::load(const std::string &book_name)
{
	// Read file contents
	const std::string file_url = OpeningBooks::book_url(book_name);
	if (!fruit::file_exists(file_url)) {
		cout << fruit::debug_description(file_url) << " does not exist" << endl;
		return;
	}
	const std::optional<std::string> optional_contents = fruit::slurp(file_url);
	if (!optional_contents.has_value()) {
		cout << "Failed to load opening book " << fruit::debug_description(file_url) << endl;
		return;
	}
	const std::string contents = optional_contents.value();
	const std::vector<std::string> lines = fruit::split(contents, '\n');
	
	// Reset
	visual_entries.clear();
	loaded = false;
	
	std::string section = "";
	std::string cache = "";
	for (const std::string &line : lines) {
		
		const std::string trimmed_line = fruit::trimming_whitespace(line);
		
		if (trimmed_line.starts_with("**")) {
			section = trimmed_line;
			continue;
		}
		if (trimmed_line.starts_with("//"))
			continue;
		
		if (section == "** VISUAL ENTRIES **") {
			cache += trimmed_line + '\n';
			if (line == "}") {
				visual_entries.emplace_back(cache);
				cache = "";
			}
		}
	}
	
	loaded = true;
	cout << "Loaded opening book " << fruit::debug_description(file_url) << endl;
}

Move OpeningBook::random_move(const AbstractGame &game) const
{
	const std::string visual = game.visual();
	for (const OpeningBook::VisualEntry &entry : visual_entries) {
		if (entry.does_match_visual(visual)) {
			return entry.random_option(game, random_engine);
		}
	}
	return NULL_MOVE;
}

void OpeningBook::sanity_check() const
{
	if (!loaded) {
		cout << "(Warning) sanity check called on unloaded book" << endl;
		return;
	}
	
	// {problem_description, visual_game_notation}
	std::vector<std::pair<std::string, std::string>> problems;
	
	auto game = AbstractGame::instantiate(CLASSIC);
	for (const VisualEntry &entry : visual_entries) {
		
		// Load the position
		game->setup_visual(entry.visual);
		// Check all options
		for (const VisualEntry::Option &option : entry.options) {
			
			Move move;
			if (option.is_universal)
				move = game->try_parse_universal(option.notation);
			else
				move = game->parse_algebraic(option.notation);
			
			const std::string notation_name = option.is_universal ? "universal" : "algebraic";
			
			// Check that the move is valid
			if (move == NULL_MOVE) {
				std::string description = fruit::debug_description(option.notation) + " could not be parsed from " + notation_name + " notation in the following position:";
				problems.emplace_back(description, entry.visual);
			}
			else if (!fruit::contains(game->legal_moves(), move)) {
				std::string description = fruit::debug_description(option.notation) + " could be parsed from " + notation_name + " notation but it is not a legal move in the following position:";
				problems.emplace_back(description, entry.visual);
			}
		}
	}
	
	if (problems.empty())
		cout << "Sanity check found no problems with opening book" << endl;
	else {
		cout << "Sanity check found the following problems with opening book:" << endl;
		cout << endl;
		for (auto &[description, visual] : problems) {
			cout << description << endl;
			cout << visual << endl;
			cout << endl;
		}
	}
}


// MARK: - Visual Entry

template<typename E>
Move OpeningBook::VisualEntry::random_option(const AbstractGame &game, E &random_engine) const
{
	// Calculate the sum of probabilities
	int total_weight = 0;
	for (const Option &option : options)
		total_weight += option.probability;
	
	// Pick a random option
	int pick = fruit::next_random<int>(random_engine, 0, total_weight - 1);
	int cumulative = 0;
	for (const Option &option : options) {
		cumulative += option.probability;
		if (cumulative > pick) {
			
			// Parse this move
			if (option.is_universal)
				return game.parse_universal(option.notation);
			return game.parse_algebraic(option.notation);
		}
	}
	fruit::fatal_error("Failed to pick random move from opening book");
}

OpeningBook::VisualEntry::VisualEntry(const std::string &string)
{
	const std::vector<std::string> lines = fruit::split(string, '\n', true);
	int index = 0;
	
	// Skip the "{"
	index++;
	
	// Parse `visual`
	while (index < lines.size() && lines[index] != "[") {
		visual += lines[index] + '\n';
		index++;
	}
	
	// Skip the "["
	if (index == lines.size())
		fruit::fatal_error("Error parsing opening book visual entry");
	index++;
	
	// Parse options
	while (index < lines.size() && lines[index] != "]") {
		
		auto tokens = fruit::tokenize(lines[index]);
		
		bool valid = false;
		bool is_universal = false;
		
		if (tokens.size() == 3 && tokens.front() == "u") {
			// Remove the starting "u"
			is_universal = true;
			tokens.erase(tokens.begin());
		}
		
		if (tokens.size() == 2) {
			const std::string &notation = tokens[0];
			try {
				std::string percent = tokens[1];
				// Drop the trailing "%" if present
				if (percent.ends_with("%"))
					percent.pop_back();
				int probability = std::stoi(percent);
				options.emplace_back(notation, is_universal, probability);
				valid = true;
			}
			catch (...) {}
		}
		if (!valid)
			fruit::fatal_error("Opening book error: visual entry contains invalid option " + fruit::debug_description(lines[index]));
		
		index++;
	}
	
	// Skip the "]"
	if (index == lines.size())
		fruit::fatal_error("Opening book error: visual entry does not contain ']'");
	index++;
	
	// Make sure the entry ends with "}"
	if (index != lines.size() - 1 || lines[index] != "}")
		fruit::fatal_error("Opening book error: visual entry does not contain ending '}'");
}

bool OpeningBook::VisualEntry::does_match_visual(const std::string &other_visual) const
{
	const std::vector<std::string> tokens = fruit::tokenize(visual);
	const std::vector<std::string> other_tokens = fruit::tokenize(other_visual);
	if (tokens.size() != other_tokens.size())
		return false;
	
	for (int index = 0; index < tokens.size(); index++) {
		if (tokens[index] == "*" || other_tokens[index] == "*")
			continue;
		if (tokens[index] != other_tokens[index])
			return false;
	}
	return true;
};


// MARK: - Option

OpeningBook::VisualEntry::Option::Option(const std::string &_notation, bool _is_universal, int _probability) : notation(_notation), is_universal(_is_universal), probability(_probability)
{}


// Explicitly instantiate the `OpeningBook` class with each variant
//template class OpeningBook<CLASSIC>;
//template class OpeningBook<ENERGY_CELLS>;
//template class OpeningBook<EXPLODING_KNIGHTS>;
//template class OpeningBook<COMPULSION>;
//template class OpeningBook<COMPULSION_AND_BACKSTABBING>;
//template class OpeningBook<PETRIFICATION>;
//template class OpeningBook<FORCED_CHECK>;
//template class OpeningBook<LOSER>;
//template class OpeningBook<KING_OF_THE_HILL>;
//template class OpeningBook<KING_OF_THE_HILL_AND_COMPULSION>;
//template class OpeningBook<THREE_CHECK>;
//template class OpeningBook<FOG_OF_WAR>;

//#define INSTANTIATE_OPENING_BOOK(VARIANT) \
//	template class OpeningBook<VARIANT>;
//
//FOR_EACH_VARIANT(INSTANTIATE_OPENING_BOOK)

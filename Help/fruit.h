//
//  fruit.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/9/22.
//

#pragma once
#ifndef bits_h
#define bits_h

#include <string>
#include <array>
#include <vector>
#include <map>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

using std::cin;
using std::cout;
using std::endl;

namespace fruit
{

void init();

// MARK: - Strings

std::string string(char ch);

std::vector<std::string> split(const std::string &string, char delimiter, bool discard_empty_subsequences = true);
std::vector<std::string> split(const std::string &string, const std::function<bool(char)> &predicate, bool discard_empty_subsequences = true);
/// Splits `string` on whitespaces, discarding empty subsequences.
std::vector<std::string> tokenize(const std::string &string);

std::string join(const std::vector<std::string> &strings, const std::string &separator);

std::string replacing_characters(const std::string &string, const std::string &old, const std::string &substitution);
std::string repeating_string(const std::string &string, int count);

/// Removes all leading and trailing whitespace.
std::string trimming_whitespace(const std::string &string);

/// Removes all characters that satisfy a given predicate.
template<class UnaryPredicate>
std::string removing_characters_where(const std::string &string, UnaryPredicate predicate)
{
	std::string result;
	for (char ch : string) {
		if (!predicate(ch))
			result += ch;
	}
	return result;
}

std::string to_lowercase(const std::string &string);
std::string to_uppercase(const std::string &string);
/// Capitalizes the first letter of the first word of `string`.
std::string capitalize_first_word(const std::string &string);

std::string debug_description(const std::string &string);
std::string debug_description(char ch);

/// Reads the entire contents of the input stream.
std::string slurp(std::ifstream &stream);
/// Reads the entire contents of the URL, returning "" if there's an error.
std::optional<std::string> slurp(const std::string &file_url);
/// Returns `true` if a file exists at `file_url`.
bool file_exists(const std::string &file_url);


// MARK: - std::vector

template<typename IN, typename OUT>
std::vector<OUT> map(const std::vector<IN> &v, std::function<OUT(const IN &)> operation)
{
	std::vector<OUT> result;
	result.reserve(v.size());
	for (const IN &element : v)
		result.push_back(operation(element));
	return result;
}

template<class T, class UnaryPredicate>
std::vector<T> filter(std::vector<T> &v, UnaryPredicate predicate);

template<class T>
bool contains(const std::vector<T> &v, T value)
{
	return std::find(v.begin(), v.end(), value) != v.end();
}

template<class T>
std::vector<T> merge(std::vector<T> &first, const std::vector<T> &last);


// MARK: - Numbers

template<class N>
std::string thousands_separated_by_commas(N number);


// MARK: - Input & Output

__attribute__((noreturn))
void fatal_error(const std::string &error);


// MARK: - Random

template<typename T, typename E>
T next_random(E &engine, T lower_bound, T upper_bound)
{
	auto distribution = std::uniform_int_distribution<T>(lower_bound, upper_bound);
	return distribution(engine);
}
template<typename T, typename E>
T next_random(E &engine)
{
	return next_random(engine, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
}


// MARK: - Operating System

std::string execute(const std::string &command, const std::function<void(std::string)> &output_line_processor);
std::string execute(const std::string &command, const std::vector<std::string> &supplemental_commands = {}, const std::function<void(std::string)> &output_line_processor = {});


// MARK: - Stopwatch

class Stopwatch
{
public:
	using clock_type = std::chrono::steady_clock;
	using second_type = std::chrono::duration<double, std::ratio<1>>;
	using time_point = std::chrono::time_point<clock_type, second_type>;
	time_point start_time;
	
	void start();
	double check() const;
	
};


// MARK: - Dispatch Queue

class DispatchQueue
{
private:
	using void_function = std::function<void()>;
	
	struct Task
	{
		void_function func;
		/// The time, in seconds since the dispatch queue's creation time, when this task should be executed.
		double deadline;
		int task_id;
		
		Task(void_function _func, double _deadline, int _task_id) : func(_func), deadline(_deadline), task_id(_task_id)
		{}
		
		struct Later
		{
			bool operator () (const Task &a, const Task &b)
			{
				return a.deadline > b.deadline;
			}
		};
	};
	
	std::string label;
	std::thread background_thread;
	std::priority_queue<Task, std::vector<Task>, Task::Later> tasks;
	
	Stopwatch stopwatch;
	int task_count;
	std::mutex mutex;
	std::condition_variable condition;
	bool stop;
	
public:
	DispatchQueue(const std::string &_label);
	~DispatchQueue();
	
	void async(const void_function &func);
	void async_after(double delay, const void_function &func);
	
	void idle_loop();
};

} // namespace fruit

#endif /* bits_h */

//
//  fruit.cpp
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/9/22.
//

#include "fruit.h"

namespace fruit
{

void init()
{
	std::srand((unsigned int)std::time(0));
	// Display `bool` values as "true"/"false" instead of "1"/"0"
	cout.setf(std::ios::boolalpha);
}

std::string string(char ch)
{
	return std::string(1, ch);
}

std::vector<std::string> split(const std::string &string, char delimiter, bool discard_empty_subsequences)
{
	std::vector<std::string> result;
	size_t subsequence_start_index = 0, next;
	while (subsequence_start_index < string.size()) {
		next = string.find(delimiter, subsequence_start_index);
		if (next == std::string::npos)
			next = string.size();
		if (discard_empty_subsequences && next == subsequence_start_index) {
			// Discard this subsequence
		}
		else
			result.push_back(string.substr(subsequence_start_index, next - subsequence_start_index));
		subsequence_start_index = next + 1;
	}
	return result;
}
std::vector<std::string> split(const std::string &string, const std::function<bool(char)> &predicate, bool discard_empty_subsequences)
{
	std::vector<std::string> result;
	size_t subsequence_start_index = 0;
	for (size_t i = 0; i <= string.size(); i++) {
		if (i == string.size() || predicate(string[i])) {
			if (discard_empty_subsequences && i == subsequence_start_index) {
				// Discard this subsequence
			}
			else
				result.push_back(string.substr(subsequence_start_index, i - subsequence_start_index));
			subsequence_start_index = i + 1;
		}
	}
	return result;
}
std::vector<std::string> tokenize(const std::string &string)
{
	std::vector<std::string> result;
	int buffer_start = 0;
	
	for (int index = 0; index < string.size(); index++) {
		if (std::isspace(string[index])) {
			if (buffer_start != index)
				result.push_back(string.substr(buffer_start, index - buffer_start));
			buffer_start = index + 1;
		}
	}
	if (buffer_start != string.size())
		result.push_back(string.substr(buffer_start));
	return result;
}

std::string join(const std::vector<std::string> &strings, const std::string &separator)
{
	std::string result = "";
	for (int a = 0; a < strings.size(); a++) {
		result += strings[a];
		if (a < strings.size() - 1)
			result += separator;
	}
	return result;
}

std::string replacing_characters(const std::string &string, const std::string &old, const std::string &substitution)
{
	std::string result;
	size_t i = 0, next;
	while (i < string.size()) {
		next = string.find(old, i);
		if (next == std::string::npos) {
			result += string.substr(i);
			break;
		}
		result += string.substr(i, next - i);
		result += substitution;
		i = next + old.size();
	}
	return result;
}
std::string repeating_string(const std::string &string, int count)
{
	std::string result = "";
	for (int a = 0; a < count; a++)
		result += string;
	return result;
}

std::string trimming_whitespace(const std::string &string)
{
	size_t start, end;
	for (start = 0; start < string.size(); start++) {
		if (!std::isspace(string[start]))
			break;
	}
	for (end = string.size(); end --> 0;) {
		if (!std::isspace(string[end]))
			break;
	}
	if (start > end)
		return "";
	return string.substr(start, end - start + 1);
}

std::string to_lowercase(const std::string &string)
{
	std::string lowercase = string;
	for (int i = 0; i < string.length(); i++)
		lowercase[i] = tolower(string[i]);
	return lowercase;
}
std::string to_uppercase(const std::string &string)
{
	std::string uppercase = string;
	for (int i = 0; i < string.length(); i++)
		uppercase[i] = toupper(string[i]);
	return uppercase;
}
std::string capitalize_first_word(const std::string &string)
{
	for (int i = 0; i < string.length(); i++)
		if (std::isalnum(string[i]))
			// This is the first letter of the first word
			return string.substr(0, i) + (char)std::toupper(string[i]) + string.substr(i + 1);
	return string;
}

std::string debug_description(const std::string &string)
{
	std::string result = "";
	for (char ch : string) {
		result += debug_description(ch);
	}
	return "\"" + result + "\"";
}
std::string debug_description(char ch)
{
	switch (ch) {
		case '\n':
			return "\\n";
		case '\t':
			return "\\t";
		default:
			return std::string(1, ch);
	}
}

std::string slurp(std::ifstream &stream)
{
	std::ostringstream converter;
	converter << stream.rdbuf();
	return converter.str();
}
std::optional<std::string> slurp(const std::string &file_url)
{
	std::ifstream input_stream;
	input_stream.open(file_url);
	if (!input_stream.is_open()) {
		return std::nullopt;
	}
	return slurp(input_stream);
}
bool file_exists(const std::string &file_url)
{
	const std::filesystem::path path(file_url);
	return std::filesystem::exists(path);
}


// MARK: - std::vector

template<class T, class UnaryPredicate>
std::vector<T> filter(std::vector<T> &v, UnaryPredicate predicate)
{
	std::vector<T> result;
	for (T value : v) {
		if (predicate(value))
			result.push_back(value);
	}
	return result;
}
template<class T>
void merge(std::vector<T> &first, const std::vector<T> &last)
{
	for (T element : last)
		first.push_back(element);
}


// MARK: - Numbers

template<class N>
std::string thousands_separated_by_commas(N number)
{
	if (number == 0) return "0";
	std::string result = "";
	int digit_count = 0;
	if (number < 0) {
		result += "-";
		number *= -1;
	}
	
	while (number) {
		// Digit
		result = std::to_string(number % 10) + result;
		number /= 10;
		// Comma
		digit_count += 1;
		if (digit_count % 3 == 0 && number != 0)
			result = "," + result;
	}
	return result;
}

// Explicity instantiate all necessary versions of this function
template std::string thousands_separated_by_commas<int>(int number);
template std::string thousands_separated_by_commas<uint64_t>(uint64_t number);


// MARK: - Input & Output

void fatal_error(const std::string &error)
{
	cout << error << endl;
	exit(1);
}


// MARK: - Operating System

std::string execute(const std::string &command, const std::function<void(std::string)> &output_line_processor)
{
	return execute(command, {}, output_line_processor);
}
std::string execute(const std::string &command, const std::vector<std::string> &supplemental_commands, const std::function<void(std::string)> &output_line_processor)
{
	char buffer[128];
	std::string result = "";
	std::string buffer_string;
	// The current line of the process's output that is being processed
	std::string current_line = "";
	
	FILE *pipe = popen(command.c_str(), "r+");
	
	// Execute supplemental commands
	for (std::string supplemental : supplemental_commands) {
		supplemental += '\n';
		fputs(supplemental.c_str(), pipe);
	}
	
	if (!pipe)
		throw std::runtime_error("popen() failed!");
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			
			buffer_string = std::string(buffer);
			result += buffer_string;
			
			// Search for newline character
			size_t newline_index;
			while ((newline_index = buffer_string.find('\n')) != std::string::npos) {
				
				current_line += buffer_string.substr(0, newline_index);
				// Process the whole line
				if (output_line_processor)
					output_line_processor(current_line);
				
				current_line = "";
				buffer_string = buffer_string.substr(newline_index + 1);
			}
			
			// Add the remaining buffer to the current line
			current_line += buffer_string;
		}
	}
	catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	return result;
}


// MARK: - Stopwatch

void Stopwatch::start()
{
	start_time = clock_type::now();
}
/// Returns the time, in seconds, that has passed since `start()` was called.
double Stopwatch::check() const
{
	return std::chrono::duration_cast<second_type>(clock_type::now() - start_time).count();
}


// MARK: - Dispatch Queue

DispatchQueue::DispatchQueue(const std::string &_label) : label(_label), stop(false), task_count(0)
{
	background_thread = std::thread(&DispatchQueue::idle_loop, this);
	stopwatch.start();
}
DispatchQueue::~DispatchQueue()
{
	// Notify the background thread that it should finish
	std::unique_lock<std::mutex> lock(mutex);
	stop = true;
	condition.notify_all();
	lock.unlock();
	
	// Wait for thread to finish
	if (background_thread.joinable())
		background_thread.join();
}

void DispatchQueue::async(const void_function &func)
{
	async_after(0, func);
}
void DispatchQueue::async_after(double delay, const void_function &func)
{
	std::unique_lock<std::mutex> lock(mutex);
	double deadline = stopwatch.check() + delay;
	Task task(func, deadline, task_count++);
	
	tasks.push(task);
	condition.notify_one();
}

void DispatchQueue::idle_loop()
{
	std::unique_lock<std::mutex> lock(mutex);
	
	while (!stop) {
		
		if (tasks.size()) {
			
			int front_task_id = tasks.top().task_id;
			Stopwatch::second_type duration(tasks.top().deadline);
			
			condition.wait_until(lock, stopwatch.start_time + duration, [this, front_task_id]() {
				// Stop waiting if `stop` is `true` or if we are out of tasks
				if (stop || tasks.empty())
					return true;
				// Stop waiting if the front of the queue changed
				return tasks.top().task_id != front_task_id;
			});
			
			if (tasks.size() && !stop) {
				
				auto task = tasks.top();
				tasks.pop();
				// We finished editing `tasks`, so we can unlock
				lock.unlock();
				
				task.func();
				
				// Re-lock before the next loop iteration
				lock.lock();
			}
		}
		else {
			condition.wait(lock, [this]() {
				// Stop waiting if `stop` is `true` or if a task is on the queue
				return stop || tasks.size();
			});
		}
	}
}

// Helpful links for dispatch queue:
// https://codereview.stackexchange.com/questions/78771/c14-async-task-scheduler
// https://stackoverflow.com/questions/11865460/issue-when-scheduling-tasks-using-clock-function?noredirect=1&lq=1

} // namespace fruit

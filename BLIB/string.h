#pragma once
#include <string>
#include "cereal.h"

/*
	The custom string class uses the framework of std::string to make a more flexible 
	and user friendly string class. It can be automatically converted to wide, automatically
	accepts ints and floats, and has some filepath tools.

	So, things like this are possible:

	string name = "James" + " " + "Cameron";
	std::cout << name; // "James Cameron"

	sting::set_precision(1);
	std::cout << string(1, " donut costs ", 1.684, " dollars."); // "1 donut costs 1.7 dollars"

	string filepath_and_name = "data/image.png";
	string filepath = filepath_and_name.get_filepath(); // "data/"
	string filename = filepath_and_name.get_filename(); // "image.png"
	string cereal_filename = filename.replace_ext("cereal"); // "image.cereal" 
*/

class string {
private:
	static inline		unsigned int	precision		= 2;
	static constexpr	unsigned int	precision_cap	= 10;
	
	std::string data = "";
	mutable std::wstring wide_cache = L"";
	mutable bool cached = false;

	// Cache the current string as a wide string.
	void cache() const;

public:
	// Segment Construction

	string() {}
	string(const string&		o)		: data(o.data) {}
	string(const std::string&	data)	: data(data) {}
	string(const char*			data)	: data(data) {}
	string(char					data)	{ this->data = data; }
	string(const std::wstring&  data)	: string(data.c_str()) {}
	string(const wchar_t*		data);
	string(wchar_t data				)	{ wchar_t d[2] = {data, 0}; *this = string(d); }
	string(int					data);
	string(float				data, int precision = string::precision);

	// Move Constructor
	string(string&& o) noexcept : data(std::move(o.data)) {}

	// Handles int - like datatypes
	template<typename I>
	string(const I i, typename std::enable_if_t<std::is_integral<I>::value>::type* = nullptr) : string(static_cast<int>(i)) {}

	// Handles float - like datatypes
	template<typename F>
	string(const F f, typename std::enable_if_t<std::is_floating_point<F>::value>::type* = nullptr) : string(static_cast<float>(f)) {}

	string& operator= (const string&  o)			{ data  = o.data;				cached = false; return *this; }
	string& operator+=(const string&  o)			{ data += o.data;				cached = false; return *this; }
	string& operator= (		 string&& o) noexcept	{ data  = o.data;				cached = false; return *this; }

	friend string operator+ (const string& a, const string& b) { return a.data + b.data; }
	
	bool operator== (const string& o) const { return data == o.data; }
	bool operator!= (const string& o) const { return data != o.data; }
	bool operator<  (const string& o) const { return data <  o.data; }

	// Allows equivalence checks between string and string-like classes.
	template <typename S, typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, string>>>
	bool operator== (const S& o) const { return operator==(string(o)); }

	// Allows non-equivalence checks between string and string-like classes.
	template <typename S, typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, string>>>
	bool operator!= (const S& o) const { return operator!=(string(o)); }

	// Alows mixing multiple string-compatible types into one string constructor, like string(float, char, string, int)
	template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
	string(Args&&... args) { (operator+=(string(std::forward<Args>(args))), ...); }

	// Static 
	
	// Set the amount of precision used when converting floats to strings.
	static void set_precision(int p) { precision = (p < 0) ? 0 : ((p < precision_cap ? p : precision_cap)); }

	// Iterating

	// Get a forward iterator, courtesy of std::string
	auto begin	() { return data.begin	(); }
	// Get a reverse iterator, courtesy of std::string
	auto rbegin	() { return data.rbegin	(); }
	// Get the endpoint for iterating, courtesy of std::string
	auto end	() { return data.end	(); }
	// Get the endpoint for reverse iterating, courtesy of std::string
	auto rend	() { return data.rend	(); }

	// Casting
	operator std::string	() const { return data; }
	operator const char*	() const { return data.c_str(); }
	operator std::wstring	() const;
	operator const wchar_t* () const { cache(); return wide_cache.c_str(); }

	// Manually convert the string to a wide string.
	const wchar_t* wide() const { return static_cast<const wchar_t*>(*this); }

	// Filepath tools

	// Truncate the string, starting at the end and KEEPING up until the first / or \. i.e., "data/image.png" -> "image.png"
	string	get_filename	()					const;
	// Truncate the string, starting at the end and REMOVING up until the first / or \. i.e., "data/image.png" -> "data/"
	string	get_filepath	()					const;
	// Replace the extension of the string with the give extension. i.e., string("image.png").replace_ext("cereal") -> "image.cereal"
	string	replace_ext		(const string& ext) const;
	// Returns True if the file exists.
	bool	file_exists		()					const;

	// Other
	char& operator[](int i) { return data[i]; }
	size_t length() const { return data.length(); }
	void pop_back() { if (!data.empty()) data.pop_back(); }

	SERIALIZE(data)
};

// Allow strings to be used as keys in dictionaries

namespace std {
	template<>
	struct hash<::string> {
		size_t operator()(const ::string& s) const noexcept {
			return std::hash<std::string>()(s);
		}
	};
}
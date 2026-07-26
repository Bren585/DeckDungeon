#pragma once
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/string.hpp>

#define NVP(obj, name) cereal::make_nvp(#name, obj.name)

// This macro registers all of the members of a class to be serialized.
#define SERIALIZE(...)		template <class Ar> inline void serialize(Ar& ar) { ar(__VA_ARGS__); }

// This macro prepares a class with no members of its own for serialization (i.e., a derived class).
#define SERIALIZE_EMPTY()	template <class Ar> inline void serialize(Ar& ar) { }

// This macro registers all of the members of a class to be serialized, and also registers it as a base class.
#define SERIALIZE_BASE(base, ...)	template <class Ar> inline void serialize(Ar& ar) { ar(cereal::base_class<base>(this)); ar(__VA_ARGS__); }

// This macro registers a class as a base class without registering any members.
#define SERIALIZE_EMPTY_BASE(base)	template <class Ar> inline void serialize(Ar& ar) { ar(cereal::base_class<base>(this)); }

// This macro can be used to force construction of a registered class instead of skipping it.
#define CONSTRUCT(type, ...)										\
namespace cereal {													\
	template <class Ar>												\
	inline void load_and_construct(Ar& ar, construct<type>& co) {	\
	type* _ptr = static_cast<type*>(::operator new(sizeof(type)));	\
	co(_ptr);														\
	auto& _ref = *_ptr;												\
	[&]() {ar(__VA_ARGS__); }();									\
} }	

// This macro can be used to force construction of a registered base class instead of skipping it.
#define CONSTRUCT_BASE(type, base, ...)									\
namespace cereal {														\
	template <class Ar>													\
	inline void load_and_construct(Ar& ar, construct<type>& co) {		\
		OutputDebugStringW(L#type);										\
		type* _ptr = static_cast<type*>(::operator new(sizeof(type)));	\
		co(_ptr);														\
		auto& self = *_ptr;												\
		[&]() {ar(base_class<base>(&self), __VA_ARGS__); }();			\
} }	

#include <fstream>
// Save a registered object to a .cereal file.
#define CEREAL(filepath, ...)	std::ofstream ofs((const char*)filepath, std::ios::binary); cereal::BinaryOutputArchive BOA(ofs); BOA(__VA_ARGS__)
// Load a registered object from a .cereal file.
#define UNCEREAL(filepath, ...)	std::ifstream ifs((const char*)filepath, std::ios::binary); cereal::BinaryInputArchive  BIA(ifs); BIA(__VA_ARGS__)
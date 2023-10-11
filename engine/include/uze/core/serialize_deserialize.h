#pragma once

#include "uze/common.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

template <class T>
struct BinarySerializer
{
	void operator()(std::ostream& o, const T& obj) const
	{
		static_assert(false, "BinarySerializer is not implemented for T");
	}
};

template <class T>
struct BinaryDeserializer
{
	void operator()(std::istream& i, T& obj) const
	{
		static_assert(false, "BinaryDeserializer is not implemented for T");
	}
};

#define DEFAULT_SERIALIZER_DESERIALIZER(T) \
	template <> struct BinarySerializer<T> \
	{ \
		void operator()(std::ostream& o, const T& obj) const \
		{ o.write(reinterpret_cast<const char*>(&obj), sizeof(T)); } \
	}; \
	 \
	template <> struct BinaryDeserializer<T> \
	{ \
		void operator()(std::istream& i, T& obj) const \
		{ i.read(reinterpret_cast<char*>(&obj), sizeof(T)); } \
	}

DEFAULT_SERIALIZER_DESERIALIZER(uze::i8);
DEFAULT_SERIALIZER_DESERIALIZER(uze::i16);
DEFAULT_SERIALIZER_DESERIALIZER(uze::i32);
DEFAULT_SERIALIZER_DESERIALIZER(uze::i64);

DEFAULT_SERIALIZER_DESERIALIZER(uze::u8);
DEFAULT_SERIALIZER_DESERIALIZER(uze::u16);
DEFAULT_SERIALIZER_DESERIALIZER(uze::u32);
DEFAULT_SERIALIZER_DESERIALIZER(uze::u64);

DEFAULT_SERIALIZER_DESERIALIZER(float);
DEFAULT_SERIALIZER_DESERIALIZER(double);

template <>
struct BinarySerializer<std::string>
{
	void operator()(std::ostream& o, const std::string& s) const
	{
		BinarySerializer<std::string::size_type>{}(o, s.size());
		o.write(s.data(), s.size());
	}
};

template <>
struct BinaryDeserializer<std::string>
{
	void operator()(std::istream& i, std::string& s) const
	{
		std::string::size_type size;
		BinaryDeserializer<std::string::size_type>{}(i, size);
		s.resize(size);
		i.read(s.data(), size);
	}
};

template <class T>
struct BinarySerializer<std::vector<T>>
{
	void operator()(std::ostream& o, const std::vector<T>& v) const
	{
		BinarySerializer<typename std::vector<T>::size_type>{}(o, v.size());
		for (const auto& e : v)
		{
			BinarySerializer<T>{}(o, e);
		}
	}
};

template <class T>
struct BinaryDeserializer<std::vector<T>>
{
	void operator()(std::istream& i, std::vector<T>& v) const
	{
		typename std::vector<T>::size_type size;
		BinaryDeserializer<typename std::vector<T>::size_type>{}(i, size);
		v.resize(size);
		for (decltype(size) n = 0; n < size; ++n)
		{
			BinaryDeserializer<T>{}(i, v[n]);
		}
	}
};

template <class K, class T>
struct BinarySerializer<std::unordered_map<K, T>>
{
	void operator()(std::ostream& o, const std::unordered_map<K, T>& m) const
	{
		BinarySerializer<typename std::unordered_map<K, T>::size_type>{}(o, m.size());
		for (auto it = m.begin(); it != m.end(); ++it)
		{
			BinarySerializer<K>{}(o, it->first);
			BinarySerializer<T>{}(o, it->second);
		}
	}
};
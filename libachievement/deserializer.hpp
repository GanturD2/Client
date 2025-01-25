#pragma once

#include "stdafx.h"
#include "stream.hpp"

#include <sstream>
#include <string>

SERIALIZE_NS_BEGIN

class deserializer {
protected:
	basic_reader& reader_;
	uint32_t size_;

public:
	deserializer(basic_reader& reader)
		: reader_(reader)
		, size_(0)
	{}

	deserializer(const deserializer&) = delete;
	~deserializer() = default;

	template <typename T>
	auto read(T& value) ->
		typename std::enable_if<std::is_arithmetic<T>::value>::type {
		reader_.read(reinterpret_cast<char*>(&value), sizeof(T));
		size_ += sizeof(T);
	}

	template <typename Elem, typename Traits, typename Alloc>
	void read(std::basic_string<Elem, Traits, Alloc>& value) {
		auto count = read<size_t>();

		value = std::move(std::string(count, '\0'));
		reader_.read(&value[0], count);
		size_ += count;
	}

	template <typename T, std::size_t Size>
	void read(std::array<T, Size>& value) {
		for (std::size_t i = 0; i < Size; ++i) {
			value[i] = read<T>();
		}
	}

	template <typename T>
	void read(std::vector<T>& value) {
		auto count = read<size_t>();

		while (count > 0) {
			value.emplace_back(std::move(read<T>()));
			--count;
		}
	}

	template <typename K, typename V>
	void read(std::map<K, V>& value) {
		auto count = read<size_t>();

		while (count > 0) {
			K key;
			read(key);

			V val;
			read(val);

			value.emplace(std::move(key), std::move(val));
			--count;
		}
	}

	template <typename T1, typename T2>
	void read(std::pair<T1, T2>& value) {
		value.first = std::move(read<T1>());
		value.second = std::move(read<T2>());
	}

	template <size_t N>
	struct tuple_helper {
		template <typename... Args>
		static void read(deserializer& self, std::tuple<Args...>& value) {
			tuple_helper<N - 1>::read(self, value);
			self.read(std::get<N - 1>(value));
		}
	};

	template <typename... Args>
	void read(std::tuple<Args...>& value) {
		tuple_helper<sizeof...(Args)>::read(*this, value);
	}

	template <typename T>
	auto read(T& sequence) ->
		typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type {
		boost::fusion::for_each(sequence, [this](auto& value) {
			read(value);
		});
	}

	template <typename T>
	T read() {
		T value;
		read(value);
		return std::move(value);
	}

	uint32_t size() const {
		return size_;
	}

	template <typename... Params>
	deserializer& operator>>(Params&&... args) {
		read(std::forward<Params>(args)...);
		return *this;
	}
};

template <>
struct deserializer::tuple_helper<0> {
	template <typename... Args>
	static void read(deserializer& self, std::tuple<Args...>& value) { }
};

template <typename T>
T deserialize(const char* buffer,
			  uint32_t bufferSize = sizeof(T),
			  uint32_t* objSize = nullptr) {
	T obj;

	std::string data(buffer, bufferSize);
	std::istringstream input(data, std::ios::binary);
	istream_wrapper wrapper(input);
	deserializer deserializer(wrapper);
	deserializer >> obj;
	
	if (objSize)
		*objSize = deserializer.size();

	return std::move(obj);
}

SERIALIZE_NS_END

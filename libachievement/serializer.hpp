#pragma once

#include "stdafx.h"
#include "io.hpp"

#include <sstream>
#include <vector>
#include "stream.hpp"

SERIALIZE_NS_BEGIN

class serializer {
protected:
	basic_writer& writer_;

public:
	serializer(basic_writer& writer)
		: writer_(writer)
	{}

	serializer(const serializer&) = delete;
	~serializer() = default;

	template <typename T>
	auto write(const T& value) ->
		typename std::enable_if<std::is_arithmetic<T>::value>::type {
		writer_.write(reinterpret_cast<const char*>(&value), sizeof(T));
	}

	template <typename Elem, typename Traits, typename Alloc>
	void write(const std::basic_string<Elem, Traits, Alloc>& value) {
		write(static_cast<size_t>(value.size()));

		writer_.write(value.c_str(), value.size());
	}

	template <typename T, std::size_t Size>
	void write(const std::array<T, Size>& value) {
		for (const auto& val : value) {
			write(val);
		}
	}

	template <typename T>
	void write(const std::vector<T>& value) {
		write(static_cast<size_t>(value.size()));

		for (const auto& val : value) {
			write(val);
		}
	}

	template <typename K, typename V>
	void write(const std::map<K, V>& value) {
		write(static_cast<size_t>(value.size()));

		for (const auto& pair : value) {
			write(pair.first);
			write(pair.second);
		}
	}

	template <size_t N>
	struct tuple_helper {
		template <typename... Args>
		static void write(serializer& self, const std::tuple<Args...>& value) {
			tuple_helper<N - 1>::write(self, value);
			self.write(std::get<N - 1>(value));
		}
	};

	template <typename... Args>
	void write(const std::tuple<Args...>& value) {
		tuple_helper<sizeof...(Args)>::write(*this, value);
	}

	template <typename T>
	auto write(const T& value) ->
		typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type {
		boost::fusion::for_each(value, [this](const auto& value) {
			write(value);
		});
	}

	template <typename... Params>
	serializer& operator<<(Params&&... args) {
		write(std::forward<Params>(args)...);
		return *this;
	}
};

template <>
struct serializer::tuple_helper<0> {
	template <typename... Args>
	static void write(serializer& self, const std::tuple<Args...>& value) { }
};

template <typename T>
std::vector<char> serialize(const T& data)
{
	typename std::ostringstream ssbuffer(std::ios::binary);
	ostream_wrapper wrapper(ssbuffer);
	serializer serializer(wrapper);
	serializer << data;

	const auto sbuffer = ssbuffer.str();
	std::vector<char> buffer(std::begin(sbuffer), std::end(sbuffer));

	return std::move(buffer);
}

SERIALIZE_NS_END

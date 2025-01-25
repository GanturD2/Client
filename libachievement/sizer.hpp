#pragma once

#include "stdafx.h"

SERIALIZE_NS_BEGIN

class sizer {
protected:
	uint32_t size_;

public:
	sizer()
		: size_(0)
	{}

	sizer(const sizer&) = delete;
	~sizer() = default;
	
	uint32_t size() const { return size_; }

	template <typename T>
	auto size(const T& value) ->
		typename std::enable_if<std::is_arithmetic<T>::value>::type {
		size_ += sizeof(T);
	}

	template <typename Elem, typename Traits, typename Alloc>
	void size(const std::basic_string<Elem, Traits, Alloc>& value) {
		size(value.size());

		size_ += value.size();
	}

	template <typename T, std::size_t Size>
	void size(const std::array<T, Size>& value) {
		for (const auto& val : value) {
			size(val);
		}
	}

	template <typename T>
	void size(const std::vector<T>& value) {
		size(value.size());

		for (const auto& val : value) {
			size(val);
		}
	}

	template <typename K, typename V>
	void size(const std::map<K, V>& value) {
		size(value.size());

		for (const auto& pair : value) {
			size(pair.first);
			size(pair.second);
		}
	}

	template <size_t N>
	struct tuple_helper {
		template <typename... Args>
		static void size(sizer& self, const std::tuple<Args...>& value) {
			tuple_helper<N - 1>::size(self, value);
			self.size(std::get<N - 1>(value));
		}
	};

	template <typename... Args>
	void size(const std::tuple<Args...>& value) {
		tuple_helper<sizeof...(Args)>::size(*this, value);
	}

	template <typename T>
	auto size(const T& value) ->
		typename std::enable_if<boost::fusion::traits::is_sequence<T>::value>::type {
		boost::fusion::for_each(value, [this](const auto& value) {
			size(value);
		});
	}

	template <typename... Params>
	void operator<<(Params&&... args) {
		size(std::forward<Params>(args)...);
	}
};

template <>
struct sizer::tuple_helper<0> {
	template <typename... Args>
	static void size(sizer& self, const std::tuple<Args...>& value) { }
};

template <typename T>
uint32_t size(const T& data) {
	sizer sizer;
	sizer.size(data);
	return sizer.size();
}

SERIALIZE_NS_END

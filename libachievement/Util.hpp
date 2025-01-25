#pragma once

#include "Types.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <string>

namespace xml {

namespace helper {

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline void to_number(const std::string& v, T& out) {
	out = boost::lexical_cast<T>(v);
}

inline void to_number(const std::string& v, int8_t& out) {
	int32_t n;
	to_number(v, n);
	out = static_cast<int8_t>(n);
}

inline void to_number(const std::string& v, uint8_t& out) {
	int32_t n;
	to_number(v, n);
	out = static_cast<uint8_t>(n);
}

};

inline boost::string_view GetName(const Node* node) {
	return boost::string_view(node->name(), node->name_size());
}

inline auto GetName(const Node& node) {
	return GetName(&node);
}

inline boost::string_view GetValue(const Node* node) {
	return boost::string_view(node->value(), node->value_size());
}

inline auto GetValue(const Node& node) {
	return GetValue(&node);
}

const Node* TryGetNode(const Node* node,
					   const std::string& name);

inline auto TryGetNode(const Node& node,
					   const std::string& name) {
	return TryGetNode(&node, name);
}

const Node* GetNode(const Node* node,
					const std::string& name);

inline auto GetNode(const Node& node,
					const std::string& name) {
	return GetNode(&node, name);
}

bool TryGetAttribute(const Node* node,
					 const std::string& name,
					 std::string& out);

inline auto TryGetAttribute(const Node& node,
							const std::string& name,
							std::string& out) {
	return TryGetAttribute(&node, name, out);
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  std::string& out);

inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 std::string& out) {
	return GetAttribute(&node, name, out);
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  const std::string& _default,
				  std::string& out);

inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 const std::string& _default,
						 std::string& out) {
	return GetAttribute(&node, name, _default, out);
}

bool TryGetAttribute(const Node* node,
					 const std::string& name,
					 bool& out);

inline auto TryGetAttribute(const Node& node,
							const std::string& name,
							bool& out) {
	return TryGetAttribute(&node, name, out);
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  bool& out);

inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 bool& out) {
	return GetAttribute(&node, name, out);
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  bool _default,
				  bool& out);

inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 bool _default,
						 bool& out) {
	return GetAttribute(&node, name, _default, out);
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
bool TryGetAttribute(const Node* node,
					 const std::string& name,
					 T& out) {
	std::string attribute;
	if (!TryGetAttribute(node, name, attribute))
		return false;

	try {
		helper::to_number(attribute, out);
	}
	catch (...) {
		return false;
	}

	return true;
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline auto TryGetAttribute(const Node& node,
							const std::string& name,
							T& out) {
	return TryGetAttribute(&node, name, out);
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
void GetAttribute(const Node* node,
				  const std::string& name,
				  T& out) {
	if (TryGetAttribute(node, name, out))
		return;

	
	const std::string message = "Failed to get attribute " + name + "in node " + node->name() + ".";
	throw Exception(message.c_str());
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 T& out) {
	return GetAttribute(&node, name, out);
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
void GetAttribute(const Node* node,
				  const std::string& name,
				  const T& _default,
				  T& out) {
	std::string attribute;
	if (!TryGetAttribute(node, name, attribute)) {
		out = _default;
		return;
	}

	try {
		helper::to_number(attribute, out);
	}
	catch (...) {
		out = _default;
	}
}

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
inline auto GetAttribute(const Node& node,
						 const std::string& name,
						 const T& _default,
						 T& out) {
	return GetAttribute(&node, name, _default, out);
}

};

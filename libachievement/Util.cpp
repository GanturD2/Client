#include "Util.hpp"

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <vector>

namespace xml {

inline bool StringToBool(const std::string& str) {
	if (!str.compare("0"))
		return false;

	if (boost::iequals(str, "false"))
		return false;

	return true;
}

const Node* TryGetNode(const Node* node,
					   const std::string& name) {
	return node->first_node(name.c_str());
}

const Node* GetNode(const Node* node,
					const std::string& name) {
	auto result = TryGetNode(node, name);
	if (result)
		return result;

	const std::string message = "Failed to get node " + name + "in node " + node->name() + ".";
	throw Exception(message.c_str());
}

bool TryGetAttribute(const Node* node,
					 const std::string& name,
					 std::string& out) {
	auto attribute = node->first_attribute(name.c_str());
	if (!attribute)
		return false;

	out = attribute->value();
	return true;
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  std::string& out) {
	if (TryGetAttribute(node, name, out))
		return;

	const std::string message = "Failed to get attribute " + name + "in node " + node->name() + ".";
	throw Exception(message.c_str());
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  const std::string& _default,
				  std::string& out) {
	if (TryGetAttribute(node, name, out))
		return;

	out = _default;
}

bool TryGetAttribute(const Node* node,
					 const std::string& name,
					 bool& out) {
	std::string attribute;
	if (!TryGetAttribute(node, name, attribute))
		return false;

	out = StringToBool(attribute);
	return true;
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  bool& out) {
	if (TryGetAttribute(node, name, out))
		return;


	const std::string message = "Failed to get attribute " + name + "in node " + node->name() + ".";
	throw Exception(message.c_str());
}

void GetAttribute(const Node* node,
				  const std::string& name,
				  bool _default,
				  bool& out) {
	if (TryGetAttribute(node, name, out))
		return;

	out = _default;
}

};

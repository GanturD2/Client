#pragma once

#include "rapidxml.hpp"

#include "Document.hpp"

#include <string>

namespace xml {

using Attribute = rapidxml::xml_attribute<>;
using Node = rapidxml::xml_node<>;

struct Exception
	: public std::exception {
public:
	explicit Exception(const std::string& message)
		: message_(message)
	{}

	explicit Exception(const char* message)
		: message_(message)
	{}

	virtual const char* what() const throw() {
		return message_.c_str();
	}

protected:
	std::string message_;
};

};

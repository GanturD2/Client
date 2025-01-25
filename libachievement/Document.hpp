#pragma once

#include "rapidxml.hpp"

#include <string>
#include <vector>

namespace xml {

class Document
	: public rapidxml::xml_document<> {
public:
	Document(const std::string& filename);

protected:
	std::vector<char> buffer_;
};

};

#include "Document.hpp"

#include <fstream>

namespace xml {

Document::Document(const std::string& filename) {
	// Read file into buffer.
	std::ifstream file(filename);
	buffer_ = std::vector<char>((std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());
	buffer_.push_back('\0');

	// Parse buffer.
	parse<rapidxml::parse_default>(buffer_.data());
}

};

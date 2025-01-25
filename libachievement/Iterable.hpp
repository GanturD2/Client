#pragma once

#include "Types.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>

#include <string>

namespace xml {

class IterableNode {
	class Iterator {
	public:
		Iterator(const Node* node)
			: node_(node)
		{}

		Iterator operator++() {
			node_ = node_->next_sibling();
			return *this;
		}

		bool operator!=(const Iterator& o) const {
			return node_ != o.node_;
		}

		const Node& operator*() const {
			return *node_;
		}

	protected:
		const Node* node_;
	};

public:
	IterableNode(const Node* root)
		: root_(root)
	{}

	IterableNode(const Node& root)
		: IterableNode(&root)
	{}

	Iterator begin() const { return Iterator(root_->first_node()); }
	Iterator end() const { return Iterator(nullptr); }

protected:
	const Node* root_;
};

};

#pragma once

#include <cstdint>
#include <stdexcept>

#include <array>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/support/is_sequence.hpp>

#define SERIALIZE_NAMESPACE serialization

#define SERIALIZE_NS_BEGIN namespace SERIALIZE_NAMESPACE {
#define SERIALIZE_NS_END }

#pragma once

#include "stdafx.h"

SERIALIZE_NS_BEGIN

class basic_reader {
public:
	virtual void read(char* data, size_t size) = 0;
};

class basic_writer {
public:
	virtual void write(const char* data, size_t size) = 0;
};

class basic_reader_writer
	: public basic_reader
	, public basic_writer
{};

SERIALIZE_NS_END

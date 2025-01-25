#pragma once

#include "stdafx.h"
#include "io.hpp"

#include <iostream>

SERIALIZE_NS_BEGIN

class istream_wrapper
	: public basic_reader {
public:
	istream_wrapper(std::istream& istream)
		: istream_(istream)
	{}
	
	virtual void read(char* data, size_t size) {
		istream_.read(data, size);
	}
	
protected:
	std::istream& istream_;
};

class ostream_wrapper
	: public basic_writer {
public:
	ostream_wrapper(std::ostream& ostream)
		: ostream_(ostream)
	{}
	
	virtual void write(const char* data, size_t size) {
		ostream_.write(data, size);
	}
	
protected:
	std::ostream& ostream_;
};

class iostream_wrapper
	: public basic_reader_writer {
public:
	iostream_wrapper(std::iostream& iostream)
		: iostream_(iostream)
	{}
	
	virtual void read(char* data, size_t size) {
		iostream_.read(data, size);
	}
	
	virtual void write(const char* data, size_t size) {
		iostream_.write(data, size);
	}
	
protected:
	std::iostream& iostream_;
};

SERIALIZE_NS_END

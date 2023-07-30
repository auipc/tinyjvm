#pragma once
#include <stdexcept>
#include <cstring>

class ErrnoException : public std::exception {
	public:
	virtual const char* what() const noexcept override {
		return strerror(errno);
	}
};

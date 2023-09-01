#pragma once
#include <cstring>
#include <stdexcept>

class ErrnoException : public std::exception {
  public:
	virtual const char *what() const noexcept override {
		return strerror(errno);
	}
};

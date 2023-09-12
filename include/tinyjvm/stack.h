#pragma once
#include <arpa/inet.h>
#include <stdint.h>
#include <vector>

namespace tinyJVM {

class StackUnderflowException : public std::exception {
	const char *what() { return "Stack Underflow"; }
};

class Stack {
  public:
	// FIXME replace this with a template later
	// also maybe ensure signedness of everything pushed onto the stack
	// except addresses
	void push(int32_t v);
	void push_64(int64_t);
	void dump_stack() const;
	int32_t pop();
	int64_t pop_64();
	int32_t peek() const;
	int64_t peek_64() const;
	inline size_t size() { return m_stack.size(); }

  private:
	std::vector<int32_t> m_stack;
};
}
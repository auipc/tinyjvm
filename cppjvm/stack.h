#pragma once
#include <arpa/inet.h>
#include <cstdint>
#include <vector>

class Stack {
  public:
	void push(int32_t v);
	int32_t pop();
	int32_t peek();
	inline size_t size() { return m_stack.size(); }

  private:
	std::vector<int32_t> m_stack;
};

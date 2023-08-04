#include <cassert>
#include <cppjvm/stack.h>
#include <iostream>

// Shamelessly stolen from stackoverflow
// Why doesn't LIBC have this though
#define htonll(x)                                                              \
	((1 == htonl(1))                                                           \
		 ? (x)                                                                 \
		 : ((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x)                                                              \
	((1 == ntohl(1))                                                           \
		 ? (x)                                                                 \
		 : ((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))

// FIXME What's with the htonl usage
// FIXME maybe stack items should be assosciated with primitives
void Stack::push(int32_t v) { m_stack.push_back(htonl(v)); }

void Stack::push_64(int64_t v) {
	// split the 64 bit value into two 32 bit values
	// and push them onto the stack
	// FIXME this is probably endianess dependant
	m_stack.push_back(htonl((int32_t)(v >> 32)));
	m_stack.push_back(htonl((int32_t)(v & 0xFFFFFFFF)));
}

// FIXME raise error when we try to access stuff from an empty stack
int32_t Stack::pop() {
	if (!m_stack.size())
		throw std::runtime_error("Stack is empty");
	int32_t v = ntohl(m_stack.back());
	m_stack.pop_back();
	return v;
}

int64_t Stack::pop_64() {
	if (m_stack.size() < 2)
		throw std::runtime_error("Stack is empty");

	// HACK
	int64_t v = peek_64();
	m_stack.pop_back();
	m_stack.pop_back();
	return v;
}

int32_t Stack::peek() {
	if (!m_stack.size())
		throw std::runtime_error("Stack is empty");
	int32_t v = ntohl(m_stack.back());
	return v;
}

int64_t Stack::peek_64() {
	if (m_stack.size() < 2)
		throw std::runtime_error("Stack is empty");
	// FIXME this is probably endianess dependant
	int64_t v = ntohl(m_stack.at(m_stack.size() - 2));
	v = (v << 32) | ntohl(m_stack.back());
	return v;
}

void Stack::dump_stack() {
	std::cout << "Stack dump:\n";
	for (auto &item : m_stack) {
		std::cout << "\t" << ntohl(item) << "\n";
	}
}
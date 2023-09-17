#include <assert.h>
#include <iostream>
#include <tinyjvm/Stack.h>

namespace tinyJVM {
// FIXME What's with the htonl usage
// FIXME maybe stack items should be assosciated with primitives
void Stack::push(int32_t v) { m_stack.push_back(v); }

void Stack::push_64(int64_t v) {
	// split the 64 bit value into two 32 bit values
	// and push them onto the stack
	// FIXME this is probably endianess dependant
	m_stack.push_back((int32_t)(v >> 32));
	m_stack.push_back((int32_t)(v & 0xFFFFFFFF));
}

// FIXME raise error when we try to access stuff from an empty stack
int32_t Stack::pop() {
	if (!m_stack.size())
		throw StackUnderflowException();
	int32_t v = m_stack.back();
	m_stack.pop_back();
	return v;
}

int64_t Stack::pop_64() {
	if (m_stack.size() < 2)
		throw StackUnderflowException();

	// HACK
	int64_t v = peek_64();
	m_stack.pop_back();
	m_stack.pop_back();
	return v;
}

int32_t Stack::peek() const {
	if (!m_stack.size())
		throw StackUnderflowException();
	int32_t v = m_stack.back();
	return v;
}

int64_t Stack::peek_64() const {
	if (m_stack.size() < 2)
		throw StackUnderflowException();
	// FIXME this is probably endianess dependant
	int64_t v = m_stack.at(m_stack.size() - 2);
	v = (v << 32) | m_stack.back();
	return v;
}

void Stack::dump_stack() const {
	std::cout << "Stack dump:\n";
	for (auto &item : m_stack) {
		std::cout << "\t" << item << "\n";
	}
}

}
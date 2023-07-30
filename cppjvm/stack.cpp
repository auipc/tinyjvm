#include <cassert>
#include <cppjvm/stack.h>

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

void Stack::push_64(int64_t v) { assert(!"Implement me"); }

// FIXME raise error when we try to access stuff from an empty stack
int32_t Stack::pop() {
	int32_t v = ntohl(m_stack.back());
	m_stack.pop_back();
	return v;
}

int64_t Stack::pop_64() { assert(!"Implement me"); }

int32_t Stack::peek() {
	int32_t v = ntohl(m_stack.back());
	return v;
}

int64_t Stack::peek_64() { assert(!"Implement me"); }

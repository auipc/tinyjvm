#include <cppjvm/stack.h>

// FIXME maybe stack items should be assosciated with primitives 
void Stack::push(int32_t v) {
    m_stack.push_back(htonl(v));
}

// FIXME raise error when we try to access stuff from an empty stack
int32_t Stack::pop() {
    int32_t v = ntohl(m_stack.back());
    m_stack.pop_back();
    return v;
}

int32_t Stack::peek() {
    int32_t v = ntohl(m_stack.back());
    return v;
}

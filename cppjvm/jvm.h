#pragma once
// Hate using the C++ STL
#include <cppjvm/classloader.h>
#include <cppjvm/stack.h>
#include <cstdint>
#include <stdio.h>
#include <vector>

class ClassLoader;

class JVM {
  public:
	JVM(ClassLoader *classloader);
	~JVM();
	void run();

  private:
	void istore(int32_t index, int32_t value);
	struct StackFrame {
		StackFrame *parent = nullptr;
		Stack operand_stack;
		std::vector<uint64_t> local_variables;
		static StackFrame create(StackFrame *parent) {
			auto stack_frame = StackFrame();
			stack_frame.parent = parent;
			// Add 10 empty local variables
			for (int i = 0; i < 10; i++)
				stack_frame.local_variables.push_back(0);

			if (parent != nullptr || parent != NULL) {
				parent->operand_stack.push((uint64_t)&stack_frame);
			}
			return stack_frame;
		}
	};

	bool m_exit = false;
	MethodCode operating_bytecode;
	std::vector<StackFrame> m_stack;
	StackFrame *m_current_stack_frame;

	inline StackFrame &stack_frame() { return *m_current_stack_frame; }
	inline Stack &operand_stack() {
		return m_current_stack_frame->operand_stack;
	}

	uint8_t bytecode_fetch_byte(uint8_t *code, size_t bytecode_size,
								size_t ptr);
	int16_t bytecode_fetch_short(uint8_t *code, size_t bytecode_size,
								 size_t ptr);

	void interpret_opcode(uint8_t opcode);
	uint32_t m_pc;
	ClassLoader *m_classloader;
};

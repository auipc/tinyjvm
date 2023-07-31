#pragma once
// Hate using the C++ STL
#include <cppjvm/classloader.h>
#include <cppjvm/stack.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <vector>

class ClassLoader;

class JVM {
  public:
	JVM(ClassLoader *classloader);
	~JVM();
	void run();
	inline void exit(const char *reason) {
		std::cout << "exiting: " << reason << "\n";
		m_exit = true;
	}

  private:
	void lstore(uint16_t index, int64_t value);
	void istore(uint16_t index, int32_t value);
	void return_from_method();

	struct StackFrame {
		StackFrame *parent = nullptr;
		Method operating_bytecode;
		Stack operand_stack;
		uint32_t pc;
		// FIXME use VarInt
		// We should probably create a generic class called Variable that will
		// be able to hold anything and everything.
		std::vector<int64_t> local_variables;
		static StackFrame& create(StackFrame *parent) {
			StackFrame* stack_frame = new StackFrame();
			stack_frame->parent = parent;
			// Add 10 empty local variables
			for (int i = 0; i <= 10; i++)
				stack_frame->local_variables.push_back(0);

			/*if (parent != nullptr) {
				parent->operand_stack.push((uint64_t)&stack_frame);
			}*/
			return *stack_frame;
		}
	};

	inline StackFrame &stack_frame() { return *m_current_stack_frame; }
	inline Stack &operand_stack() {
		return m_current_stack_frame->operand_stack;
	}

	uint8_t bytecode_fetch_byte(uint8_t *code, size_t bytecode_size,
								size_t ptr);
	int16_t bytecode_fetch_short(uint8_t *code, size_t bytecode_size,
								 size_t ptr);

	void interpret_opcode(uint8_t opcode);
	inline uint32_t incr_program_counter() {
		m_current_stack_frame->pc++;
		return m_current_stack_frame->pc;
	}

	inline uint32_t get_program_counter() {
		return m_current_stack_frame->pc;
	}

	inline void set_program_counter(uint32_t pc) {
		m_current_stack_frame->pc = pc;
	}

	inline void add_program_counter(uint32_t pc) {
		m_current_stack_frame->pc += pc;
	}

	bool m_exit = false;
	std::vector<StackFrame> m_stack;
	StackFrame *m_current_stack_frame;
	ClassLoader *m_classloader;
};

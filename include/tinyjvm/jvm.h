#pragma once
// Hate using the C++ STL
#include <tinyjvm/classloader.h>
#include <tinyjvm/stack.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <cassert>
#include <memory>
#include <cstring>

class ClassLoader;
class JVM;

class Variable {
  public:
	enum Tags {
		Integer = 0,
		Float = 1,
		Long = 2,
		Double = 3,
		ObjectReference = 4,
		ArrayRef = 5
	};
	Variable() {
		m_data = nullptr;
		m_is_null = true;
	}

	~Variable() {
		if (!m_is_null)
			delete[] m_data;
	}

	template <typename T> T get() {
		return *(T *)m_data;
	}

	template <typename T> T get_fault_type() {
		if (!m_data)
			throw std::runtime_error("Variable is null");

		if (m_tag == Tags::Integer != std::is_same<T, int32_t>::value)
			throw std::runtime_error("Invalid type");
		if (m_tag == Tags::Float != std::is_same<T, float>::value)
			throw std::runtime_error("Invalid type");
		if (m_tag == Tags::Long != std::is_same<T, int64_t>::value)
			throw std::runtime_error("Invalid type");
		if (m_tag == Tags::Double != std::is_same<T, double>::value)
			throw std::runtime_error("Invalid type");
		if (m_tag == Tags::ObjectReference != std::is_same<T, uintptr_t>::value)
			throw std::runtime_error("Invalid type");
		if (m_tag == Tags::ArrayRef != std::is_same<T, uintptr_t>::value)
			throw std::runtime_error("Invalid type");

		return *(T *)m_data;
	}

	template <typename T> void set(Tags new_tag, T value) {
		//if (new_tag == Tags::Long)

		if (!m_is_null) {
			if (new_tag != m_tag) {
				delete[] m_data;
				m_data = new uint8_t[sizeof(T)];
				std::memset(m_data, 0, sizeof(uint8_t) * sizeof(T));
			}
		} else {
			m_data = new uint8_t[sizeof(T)];
			std::memset(m_data, 0, sizeof(uint8_t) * sizeof(T));
			m_is_null = false;
		}

		m_tag = new_tag;
		*(T *)m_data = value;
	}

	// Disallow copying the Variable class
	Variable(const Variable &) = delete;
	Variable &operator=(const Variable &) = delete;
  private:
	Tags m_tag;
	uint8_t* m_data;
	bool m_is_null;
};

class JVM {
  public:
	JVM(ClassLoader *classloader);
	void run();
	inline void exit(const char *reason, int code=0) {
		std::cout << "exiting: " << reason << "\n";
		m_exitcode = code;
		m_exit = true;
	}

	int exitcode() { return m_exitcode; }

	void lstore(uint16_t index, int64_t value);
	void istore(uint16_t index, int32_t value);
	void jump_to(int32_t offset);
	void return_from_method();

	struct StackFrame {
		StackFrame *parent = nullptr;
		Method operating_bytecode;
		Stack operand_stack;
		uint32_t pc;
		// FIXME use VarInt
		// We should probably create a generic class called Variable that will
		// be able to hold anything and everything.
		std::vector<std::shared_ptr<Variable>> local_variables;

		StackFrame() {
			pc = 0;
			// Add empty local variables
			for (int i = 0; i <= 10; i++) {
				local_variables.push_back(std::make_shared<Variable>());
				local_variables.back()->set(Variable::Integer, 0);
			}
		}

		static StackFrame& create(StackFrame *parent) {
			StackFrame* stack_frame = new StackFrame();
			stack_frame->parent = parent;

			return *stack_frame;
		}

		void set_operating_bytecode(Method method) {
			operating_bytecode = method;

			// Add empty local variables
			if (method.max_locals > 10) {
				for (int i = 0; i < method.max_locals; i++) {
					local_variables.push_back(std::make_shared<Variable>());
					local_variables.back()->set(Variable::Integer, 0);
				}
			}
		}
	};

	inline StackFrame &stack_frame() { return *m_current_stack_frame; }
	inline Stack &operand_stack() {
		return m_current_stack_frame->operand_stack;
	}

	uint8_t fetch_byte(uint32_t pos);
	int16_t fetch_short(uint32_t pos);
	int32_t fetch_int(uint32_t pos);

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

	inline int max_locals() {
		return m_current_stack_frame->operating_bytecode.max_locals-1;
	}

	inline ClassLoader* classloader() { return m_classloader; }
	inline void set_stack_frame(StackFrame* frame) { m_current_stack_frame = frame; }

	std::vector<std::unique_ptr<Variable>> opcode_parameters;
  private:
	bool m_exit = false;
	int m_exitcode = 0;
	std::vector<StackFrame> m_stack;
	StackFrame *m_current_stack_frame;
	ClassLoader *m_classloader;
};

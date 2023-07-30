#include <arpa/inet.h>
#include <cassert>
#include <cppjvm/classloader.h>
#include <cppjvm/jvm.h>
#include <iostream>

// I think it's permissible to have bytecode and memory seperate.
/*
uint8_t bytecode[] = {
	0x1a, 0x1b, 0x60, 0x57, 0xb1
};

uint8_t bytecode_size = sizeof(bytecode) / sizeof(bytecode[0]);
*/

JVM::JVM(ClassLoader *classloader) : m_classloader(classloader), m_pc(0) {
	// auto root_stack_frame = StackFrame();
	// m_stack.push_back(root_stack_frame);
	auto root_stack_frame = StackFrame::create(nullptr);
	m_stack.push_back(root_stack_frame);
	// root stackframe
	m_current_stack_frame = &m_stack[0];
}

JVM::~JVM() {}

// TODO allocate memory for attributes of the class and figure out how to
// resolve dependencies in a clean way so we can call Object's initializer in
// case there's implementation specific quirks in the standard java lib Maybe we
// should write our own :^)
void JVM::run() {
	operating_bytecode = m_classloader->method_code[std::string("main")];
	std::cout << operating_bytecode.code_length << "\n";
	std::cout << m_pc << "\n";
	do {
		uint8_t opcode = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		interpret_opcode(opcode);
		if ((m_pc + 1) > operating_bytecode.code_length)
			m_exit = true;
	} while (!m_exit);
}

uint8_t JVM::bytecode_fetch_byte(uint8_t *code, size_t bytecode_size,
								 size_t ptr) {
	assert(ptr < bytecode_size);
	return code[ptr];
}

int16_t JVM::bytecode_fetch_short(uint8_t *code, size_t bytecode_size,
								  size_t ptr) {
	assert(ptr < bytecode_size);
	int16_t value = code[ptr] << 8 | code[ptr + 1];
	return value;
}

void JVM::istore(int32_t index, int32_t value) {
	if (index > 10) {
		throw std::runtime_error("Variable index out of bounds");
	}
	stack_frame().local_variables[index] = value;
}

void JVM::interpret_opcode(uint8_t opcode) {
	switch (opcode) {
	// NOP
	case 0:
		printf("NOP %d\n", m_pc);
		break;
	// ICONST_0
	case 0x03:
		// Push 0 onto the stack
		operand_stack().push(0);
		break;
	// ICONST_1
	case 0x04:
		operand_stack().push(1);
		break;
	// ISTORE
	case 0x36: {
		uint8_t index = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		/*if (index > 10) {
			throw std::runtime_error("Variable index out of bounds");
		}

		stack_frame().local_variables[index] = operand_stack().pop();*/
		istore(index, operand_stack().pop());
		break;
	}
	// ISTORE_0
	case 0x3b:
		istore(0, operand_stack().pop());
		break;
	// ISTORE_1
	case 0x3c:
		istore(1, operand_stack().pop());
		break;
	// ISTORE_2
	case 0x3d:
		istore(2, operand_stack().pop());
		break;
	// ISTORE_3
	case 0x3e:
		istore(3, operand_stack().pop());
		break;
	// ILOAD
	case 0x15: {
		std::cout << "ILOAD\n";
		uint8_t index = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		// FIXME bounds check
		operand_stack().push(stack_frame().local_variables[index]);
		break;
	}
	// ILOAD_0
	case 0x1a:
		operand_stack().push(stack_frame().local_variables[0]);
		break;
	// ILOAD_1
	case 0x1b:
		operand_stack().push(stack_frame().local_variables[1]);
		break;
	// ILOAD_2
	case 0x1c:
		operand_stack().push(stack_frame().local_variables[2]);
		break;
	// ILOAD_3
	case 0x1d:
		operand_stack().push(stack_frame().local_variables[3]);
		break;
	// IINC
	case 0x84: {
		uint8_t index = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		std::cout << "IINC " << (int)index << "\n";
		int8_t constant = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		stack_frame().local_variables[index] += constant;
		std::cout << "v " << stack_frame().local_variables[index] << "\n";
		break;
	}
	// GOTO
	case 0xa7: {
		int16_t offset = bytecode_fetch_short(
			operating_bytecode.code, operating_bytecode.code_length, m_pc);
		std::cout << "goto offset " << offset << "\n";
		m_pc += offset - 1;
		break;
	}
	// BIPUSH
	case 0x10: {
		uint8_t value = bytecode_fetch_byte(
			operating_bytecode.code, operating_bytecode.code_length, m_pc++);
		operand_stack().push(value);
		break;
	}
	// IF_ICMPGE
	case 0xa2: {
		int16_t offset = bytecode_fetch_short(
			operating_bytecode.code, operating_bytecode.code_length, m_pc);
		m_pc += 2;
		int32_t b = operand_stack().pop();
		int32_t a = operand_stack().pop();
		std::cout << "a: " << a << " b: " << b << "\n";
		if (a >= b) {
			m_pc += offset - 1;
		}
		break;
	}
	// RETURN
	case 0xb1:
		std::cout << "return\n";
		// If our current stackframe's parent is null we return
		if (!stack_frame().parent) {
			std::cout << "Exit from main"
					  << "\n";
			m_exit = true;
			break;
		}
		break;
	// POP
	case 0x57:
		operand_stack().pop();
		break;
	// IADD
	case 0x60: {
		int32_t a = operand_stack().pop();
		int32_t b = operand_stack().pop();
		operand_stack().push(a + b);
		std::cout << "a + b = " << operand_stack().peek() << "\n";
		break;
	}
	default:
		printf("Unknown opcode 0x%x encountered, halting...\n", opcode);
		while (1)
			;
		break;
	}
}

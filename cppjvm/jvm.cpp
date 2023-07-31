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

JVM::JVM(ClassLoader *classloader) : m_classloader(classloader) {
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
	operating_bytecode = m_classloader->methods[std::string("main")];
	for (auto &pair : m_classloader->methods) {
		std::cout << "lol\n";
		std::cout << pair.first << "\n";
		std::cout << pair.second.code_length << "\n";
		std::cout << "lol\n";
	}
	std::cout << operating_bytecode.code_length << "\n";
	std::cout << get_program_counter() << "\n";
	do {
		uint8_t opcode = bytecode_fetch_byte(operating_bytecode.code,
											 operating_bytecode.code_length,
											 get_program_counter());
		incr_program_counter();
		std::cout << "opcode: " << std::hex << (int)opcode << std::dec << "\n";
		interpret_opcode(opcode);
		if ((get_program_counter() + 1) > operating_bytecode.code_length)
			exit("End of bytecode");
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

void JVM::istore(uint16_t index, int32_t value) {
	if (index > 10) {
		throw std::runtime_error(
			"FIXME allocate more variables if we need them");
	}
	stack_frame().local_variables[index] = value;
}

void JVM::lstore(uint16_t index, int64_t value) {
	if (index > 10) {
		throw std::runtime_error(
			"FIXME allocate more variables if we need them");
	}
	stack_frame().local_variables[index] = value;
}

void JVM::interpret_opcode(uint8_t opcode) {
	switch (opcode) {
	// NOP
	case 0:
		printf("NOP %d\n", get_program_counter());
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
	// ICONST_2
	case 0x05:
		operand_stack().push(2);
		break;
	// ICONST_5
	case 0x08:
		operand_stack().push(5);
		break;
	// LCONST_0
	case 0x09:
		// Our stack really should just use VarInts lol what is this
		operand_stack().push(0L);
		break;
	// LCONST_1
	case 0x0a:
		// Our stack really should just use VarInts lol what is this
		operand_stack().push(1L);
		break;
	// ISTORE
	case 0x36: {
		uint8_t index = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
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
	// LSTORE_0
	case 0x3f:
		lstore(0, operand_stack().pop_64());
		break;
	// LSTORE
	case 0x37: {
		uint8_t index = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
		lstore(index, operand_stack().pop_64());
		break;
	}
	// LSTORE_1
	case 0x40:
		lstore(1, operand_stack().pop_64());
		break;
	// LSTORE_2
	case 0x41:
		lstore(2, operand_stack().pop_64());
		break;
	// LSTORE_3
	case 0x42:
		lstore(3, operand_stack().pop_64());
		break;
	// ILOAD
	case 0x15: {
		uint8_t index = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		std::cout << stack_frame().local_variables[index] << "\n";
		operand_stack().push(stack_frame().local_variables[index]);
		std::cout << "ILOAD " << (int)index << "\n";
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
	// LLOAD
	case 0x16: {
		uint8_t index = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		operand_stack().push_64(stack_frame().local_variables[index]);
		std::cout << "LLOAD " << (int)index << "\n";
		break;
	}
	// LLOAD_0
	case 0x1e:
		operand_stack().push_64(stack_frame().local_variables[0]);
		break;
	// LLOAD_1
	case 0x1f:
		operand_stack().push_64(stack_frame().local_variables[1]);
		break;
	// LLOAD_2
	case 0x20:
		operand_stack().push_64(stack_frame().local_variables[2]);
		break;
	// LLOAD_3
	case 0x21:
		operand_stack().push_64(stack_frame().local_variables[3]);
		break;
	// IINC
	case 0x84: {
		uint8_t index = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
		int8_t constant = bytecode_fetch_byte(operating_bytecode.code,
											  operating_bytecode.code_length,
											  get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		stack_frame().local_variables[index] += constant;
		std::cout << "IINC " << (int)index << " " << (int)constant << "\n";
		std::cout << "new_pc: " << get_program_counter() << "\n";
		break;
	}
	// GOTO
	case 0xa7: {
		int16_t offset = bytecode_fetch_short(operating_bytecode.code,
											  operating_bytecode.code_length,
											  get_program_counter());
		std::cout << "goto offset " << offset << "\n";
		add_program_counter(offset - 1);
		break;
	}
	// BIPUSH
	case 0x10: {
		uint8_t value = bytecode_fetch_byte(operating_bytecode.code,
											operating_bytecode.code_length,
											get_program_counter());
		incr_program_counter();
		std::cout << "BIPUSH " << (int)value << "\n";
		operand_stack().push(value);
		break;
	}
	// SIPUSH
	case 0x11: {
		uint16_t value = bytecode_fetch_short(
			operating_bytecode.code, operating_bytecode.code_length, get_program_counter());
		add_program_counter(2);
		operand_stack().push(value);
		break;
	}
	// IF_ICMPGE
	case 0xa2: {
		int16_t offset = bytecode_fetch_short(
			operating_bytecode.code, operating_bytecode.code_length, get_program_counter());
		add_program_counter(2);

		operand_stack().dump_stack();
		int32_t b = operand_stack().pop();
		int32_t a = operand_stack().pop();
		std::cout << "a: " << a << " b: " << b << "\n";
		if (a >= b) {
			add_program_counter(offset);
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
			exit("Exit from main");
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
	// LADD
	case 0x61: {
		int64_t a = operand_stack().pop();
		int64_t b = operand_stack().pop();
		operand_stack().push(a + b);
		std::cout << "a + b = " << operand_stack().peek() << "\n";
		break;
	}
	// INVOKESTATIC
	case 0xb8: {
		uint16_t index = bytecode_fetch_short(
			operating_bytecode.code, operating_bytecode.code_length, get_program_counter());
		add_program_counter(2);
		auto cp_entry = m_classloader->get_const_pool_entry(index);
		if (cp_entry.tag != ConstPoolTag::MethodRef) {
			std::cout << "Expected CONSTANT_Methodref, got " << cp_entry.tag
					  << "\n";
			exit("Bad constant pool entry");
		}


		break;
	}
	default:
		printf("Unknown opcode 0x%x encountered, m_pc = %d exiting...\n",
			   opcode, get_program_counter());
		fflush(stdout);
		exit("Bad opcode");
		break;
	}
}

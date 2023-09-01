#include <arpa/inet.h>
#include <cassert>
#include <cppjvm/classloader.h>
#include <cppjvm/opcodes.h>
#include <cppjvm/jvm.h>
#include <iostream>

JVM::JVM(ClassLoader *classloader) : m_classloader(classloader) {
	// auto root_stack_frame = StackFrame();
	// m_stack.push_back(root_stack_frame);
	auto root_stack_frame = StackFrame::create(nullptr);
	m_stack.push_back(root_stack_frame);
	// root stackframe
	m_current_stack_frame = &m_stack[0];
}

// TODO allocate memory for attributes of the class and figure out how to
// resolve dependencies in a clean way so we can call Object's initializer in
// case there's implementation specific quirks in the standard java lib Maybe we
// should write our own :^)
void JVM::run() {
	stack_frame().set_operating_bytecode(
		m_classloader->methods[std::string("main")]);

	do {
		uint8_t opcode =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		std::cout << "opcode: " << std::hex << (int)opcode << std::dec << "\n";
		if (Opcodes::opcode_map.contains(opcode)) {
			for (int i = 0; i < Opcodes::opcode_map[opcode].no_parameters; i++) {
				switch (Opcodes::opcode_map[opcode].parameter_type) {
					case OpcodeHandle::Byte: {
						uint8_t byte_param = bytecode_fetch_byte(stack_frame().operating_bytecode.code, 
																 stack_frame().operating_bytecode.code_length,
																 get_program_counter());
						incr_program_counter();

						auto var = std::make_unique<Variable>();
						var.get()->set(Variable::Tags::Integer, byte_param);
						opcode_parameters.push_back(std::move(var));
					} break;
					case OpcodeHandle::Short: {
						uint16_t short_param = bytecode_fetch_short(stack_frame().operating_bytecode.code, 
																 stack_frame().operating_bytecode.code_length,
																 get_program_counter());
						add_program_counter(2);

						auto var = std::make_unique<Variable>();

						// Nothing like a bit of widening
						var.get()->set(Variable::Tags::Integer, short_param);
						opcode_parameters.push_back(std::move(var));
					} break;
					default:
						exit("Bad opcode parameter", 1);
						break;
				}
			}

			Opcodes::opcode_map.at(opcode).function(*this);
		} else {
			Opcodes::Unknown(*this);
		}

		opcode_parameters.clear();

		//interpret_opcode(opcode);
		if ((get_program_counter()) >
			stack_frame().operating_bytecode.code_length)
			exit("End of bytecode", 1);
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

int32_t JVM::bytecode_fetch_int(uint8_t *code, size_t bytecode_size,
								size_t ptr) {
	assert(ptr < bytecode_size);
	int32_t value = code[ptr] << 24 | code[ptr + 1] << 16 | code[ptr + 2] << 8 |
					code[ptr + 3];
	return value;
}

void JVM::istore(uint16_t index, int32_t value) {
	if (index > max_locals()) {
		throw std::runtime_error(
			"FIXME allocate more variables if we need them");
	}
	stack_frame().local_variables[index]->set(Variable::Tags::Integer, value);
}

void JVM::lstore(uint16_t index, int64_t value) {
	if (index > max_locals()) {
		throw std::runtime_error(
			"FIXME allocate more variables if we need them");
	}
	stack_frame().local_variables[index]->set(Variable::Tags::Long, value);
}

void JVM::jump_to(int32_t offset) { add_program_counter(offset - 1); }

void JVM::return_from_method() {
	std::cout << "return\n";
	// If our current stackframe's parent is null we return
	if (!stack_frame().parent) {
		std::cout << "Exit from main"
				  << "\n";
		exit("Exit from main");
		return;
	}

	StackFrame *previous = &stack_frame();
	m_current_stack_frame = stack_frame().parent;
	delete previous;
}

/*
void JVM::interpret_opcode(uint8_t opcode) {
	switch (opcode) {
	// NOP
	case 0:
		printf("NOP %d\n", get_program_counter());
		break;
	// ICONST_M1
	case 0x02:
		// Push -1 onto the stack
		operand_stack().push(-1);
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
	// ICONST_3
	case 0x06:
		operand_stack().push(3);
		break;
	// ICONST_4
	case 0x07:
		operand_stack().push(4);
		break;
	// ICONST_5
	case 0x08:
		operand_stack().push(5);
		break;
	// LCONST_0
	case 0x09:
		// Our stack really should just use VarInts lol what is this
		operand_stack().push_64(0L);
		break;
	// LCONST_1
	case 0x0a:
		// Our stack really should just use VarInts lol what is this
		operand_stack().push_64(1L);
		std::cout << "LCONST_1 " << operand_stack().peek_64() << "\n";
		break;
	// ISTORE
	case 0x36: {
		uint8_t index =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
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
		uint8_t index =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
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
		uint8_t index =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		std::cout << stack_frame().local_variables[index]->get_fault_type<int>()
				  << "\n";
		operand_stack().push(
			stack_frame().local_variables[index]->get_fault_type<int>());
		std::cout << "ILOAD " << (int)index << "\n";
		break;
	}
	// ILOAD_0
	case 0x1a:
		operand_stack().push(
			stack_frame().local_variables[0]->get_fault_type<int>());
		break;
	// ILOAD_1
	case 0x1b:
		operand_stack().push(
			stack_frame().local_variables[1]->get_fault_type<int>());
		break;
	// ILOAD_2
	case 0x1c:
		operand_stack().push(
			stack_frame().local_variables[2]->get_fault_type<int>());
		break;
	// ILOAD_3
	case 0x1d:
		operand_stack().push(
			stack_frame().local_variables[3]->get_fault_type<int>());
		break;
	// LLOAD
	case 0x16: {
		uint8_t index =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		operand_stack().push_64(
			stack_frame().local_variables[index]->get_fault_type<int64_t>());
		std::cout << "LLOAD " << (int)index << "\n";
		break;
	}
	// LLOAD_0
	case 0x1e:
		operand_stack().push_64(
			stack_frame().local_variables[0]->get_fault_type<int64_t>());
		break;
	// LLOAD_1
	case 0x1f:
		operand_stack().push_64(
			stack_frame().local_variables[1]->get_fault_type<int64_t>());
		break;
	// LLOAD_2
	case 0x20:
		operand_stack().push_64(
			stack_frame().local_variables[2]->get_fault_type<int64_t>());
		break;
	// LLOAD_3
	case 0x21:
		operand_stack().push_64(
			stack_frame().local_variables[3]->get_fault_type<int64_t>());
		break;
	// IINC
	case 0x84: {
		uint8_t index =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		int8_t constant =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		// FIXME bounds check
		// Lengthy
		stack_frame().local_variables[index].get()->set(
			Variable::Tags::Integer,
			stack_frame().local_variables[index].get()->get_fault_type<int>() +
				constant);
		std::cout << "IINC " << (int)index << " " << (int)constant << "\n";
		std::cout << "new_pc: " << get_program_counter() << "\n";
		break;
	}
	// GOTO
	case 0xa7: {
		int16_t offset =
			bytecode_fetch_short(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
		std::cout << "goto offset " << offset << "\n";
		jump_to(offset);
		break;
	}
	// GOTO_W
	case 0xc8: {
		int32_t offset =
			bytecode_fetch_int(stack_frame().operating_bytecode.code,
							   stack_frame().operating_bytecode.code_length,
							   get_program_counter());
		jump_to(offset);
		break;
	}
	// BIPUSH
	case 0x10: {
		uint8_t value =
			bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								stack_frame().operating_bytecode.code_length,
								get_program_counter());
		incr_program_counter();
		std::cout << "BIPUSH " << (int)value << "\n";
		operand_stack().push(value);
		break;
	}
	// SIPUSH
	case 0x11: {
		uint16_t value =
			bytecode_fetch_short(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
		add_program_counter(2);
		operand_stack().push(value);
		break;
	}
	// IF_ICMPGE
	case 0xa2: {
		int16_t offset =
			bytecode_fetch_short(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
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
	// IRETURN
	case 0xac: {
		auto parent = stack_frame().parent;
		parent->operand_stack.push(operand_stack().pop());
		return_from_method();
	} break;
	case 0xb1: {
		return_from_method();
	} break;
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
	// IMUL
	case 0x68: {
		int32_t a = operand_stack().pop();
		int32_t b = operand_stack().pop();
		operand_stack().push(a * b);
		std::cout << "a * b = " << operand_stack().peek() << "\n";
		break;
	}
	// LADD
	case 0x61: {
		int64_t a = operand_stack().pop_64();
		int64_t b = operand_stack().pop_64();
		std::cout << "a: " << a << " b: " << b << "\n";
		std::cout << "a + b = " << a + b << "\n";
		operand_stack().push_64(a + b);
		std::cout << "a + b = " << operand_stack().peek_64() << "\n";
		break;
	}
	// INVOKESTATIC
	case 0xb8: {
		// TODO check if method is static
		uint16_t index =
			bytecode_fetch_short(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
		add_program_counter(2);
		auto cp_entry = m_classloader->get_const_pool_entry(index);
		if (cp_entry.tag != ConstPoolTag::MethodRef) {
			std::cout << "Expected CONSTANT_Methodref, got " << cp_entry.tag
					  << "\n";
			exit("Bad constant pool entry");
		}

		auto name_and_type =
			m_classloader->get_const_pool_entry(cp_entry.name_and_type_index);

		if (name_and_type.tag != ConstPoolTag::NameAndType) {
			std::cout << "Expected CONSTANT_NameAndType, got "
					  << name_and_type.tag << "\n";
			exit("Bad constant pool entry");
		}

		// FIXME utf8 isn't null terminated. sort of a big issue
		auto name_str =
			m_classloader->get_const_pool_entry(name_and_type.name_index);
		std::cout << name_str.utf8 << "\n";
		m_current_stack_frame = &StackFrame::create(m_current_stack_frame);
		// TODO check if the method matches the type signature
		// otherwise the stack will be corrupted and we won't find the function
		// we're looking for.
		stack_frame().set_operating_bytecode(
			m_classloader->methods[std::string(name_str.utf8)]);

		break;
	}
	// NEWARRAY
	case 0xbc:
	*/
		/* The correspondence between type codes and primitive types is specified by the following predicate:
		 * primitiveArrayInfo(4,  0'Z, boolean, int).
		 * primitiveArrayInfo(5,  0'C, char,    int).
		 * primitiveArrayInfo(6,  0'F, float,   float).
		 * primitiveArrayInfo(7,  0'D, double,  double).
		 * primitiveArrayInfo(8,  0'B, byte,    int).
		 * primitiveArrayInfo(9,  0'S, short,   int).
		 * primitiveArrayInfo(10, 0'I, int,     int). 
		 * primitiveArrayInfo(11, 0'J, long,    long).
		 */
		/*	   {
		uint8_t primitive_index = bytecode_fetch_byte(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
		add_program_counter(1);

		std::cout << "primitive_index " << (int)primitive_index << "\n";
		assert(false);
			   }
		break;
	// LDC2_W
	case 0x14: {
		uint16_t index =
			bytecode_fetch_short(stack_frame().operating_bytecode.code,
								 stack_frame().operating_bytecode.code_length,
								 get_program_counter());
		add_program_counter(2);
		auto cp_entry = m_classloader->get_const_pool_entry(index);
		if (cp_entry.tag != ConstPoolTag::Long) {
			std::cout << "Expected CONSTANT_Long, got " << cp_entry.tag << "\n";
			exit("Bad constant pool entry");
		}
		std::cout << "Pushing long " << cp_entry.numbers.long_integer << "\n";
		operand_stack().push_64(cp_entry.numbers.long_integer);
		std::cout << "Long pushed " << operand_stack().peek_64() << "\n";
		break;
	}
	default:
		printf("Unknown opcode 0x%x encountered, m_pc = %d exiting...\n",
			   opcode, get_program_counter());
		fflush(stdout);
		exit("Bad opcode", 1);
		break;
	}
}*/

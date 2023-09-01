#include <arpa/inet.h>
#include <cassert>
#include <tinyjvm/classloader.h>
#include <tinyjvm/opcodes.h>
#include <tinyjvm/jvm.h>
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
// case there's implementation specific quirks in the standard java lib. Maybe we
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

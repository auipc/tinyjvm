#include <arpa/inet.h>
#include <assert.h>
#include <tinyjvm/ClassLoader.h>
#include <tinyjvm/Opcodes.h>
#include <tinyjvm/JVM.h>
#include <iostream>

namespace tinyJVM {
size_t JVM::s_arrayrefs = 0;

JVM::JVM(ClassLoader *classloader) : m_classloader(classloader) {
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
			fetch_byte(get_program_counter());
		incr_program_counter();
		std::cout << "opcode: " << std::hex << (int)opcode << std::dec << "\n";
		if (Opcodes::opcode_map.contains(opcode)) {
			for (int i = 0; i < Opcodes::opcode_map[opcode].no_parameters; i++) {
				switch (Opcodes::opcode_map[opcode].parameter_type) {
					case OpcodeHandle::Byte: {
						uint8_t byte_param = fetch_byte(get_program_counter());
						incr_program_counter();

						auto var = std::make_unique<Variable>();
						var.get()->set(Variable::Tags::Integer, byte_param);
						opcode_parameters.push_back(std::move(var));
					} break;
					case OpcodeHandle::Short: {
						uint16_t short_param = fetch_short(get_program_counter());
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

uint8_t JVM::fetch_byte(uint32_t pos) {
	auto code = stack_frame().operating_bytecode;
	assert(pos < code.code_length);
	return code.code[pos];
}

int16_t JVM::fetch_short(uint32_t pos) {
	auto code = stack_frame().operating_bytecode;
	assert(pos < code.code_length);
	int16_t value = code.code[pos] << 8 | code.code[pos + 1];
	return value;
}

int32_t JVM::fetch_int(uint32_t pos) {
	auto code = stack_frame().operating_bytecode;
	assert(pos < code.code_length);
	int32_t value = code.code[pos] << 24 | code.code[pos + 1] << 16 | code.code[pos + 2] << 8 |
					code.code[pos + 3];
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

void JVM::collect_garbage() {
	// TODO after we reach zero refs, we should put the object on a queue so searching for unused objects isn't long.
	for (auto it = m_arrayrefs.cbegin(); it != m_arrayrefs.cend();) {
		if (!it->second) {
			it++;
			continue;
		}

		if (!it->second->refcount()) {
			std::cout << "Deleting object\n";
			delete it->second;
			m_arrayrefs.erase(it++);
			continue;
		}
		it++;
	}
}

void JVM::return_from_method() {
	std::cout << "return\n";

	// Deref all objects after we leave the method
	for (auto object : stack_frame().arrays_owned) {
		m_arrayrefs[object]->deref();
	}

	// invoke GC
	collect_garbage();

	// If our current stackframe's parent is null we return
	if (!stack_frame().parent) {
		exit("Exit from main");
		return;
	}

	StackFrame &previous = stack_frame();
	m_current_stack_frame = stack_frame().parent;

	// This sucks
	delete &previous;
}
}
#include <tinyjvm/opcodes.h>
#include <tinyjvm/jvm.h>

void Opcodes::NOP(JVM& context) {
	std::cout << "opcode\n";
}

void Opcodes::INVOKESTATIC(JVM& context) {
	int index = context.opcode_parameters.at(0).get()->get_fault_type<int>();

	auto cp_entry = context.classloader()->get_const_pool_entry(index);
	if (cp_entry.tag != ConstPoolTag::MethodRef) {
		std::cout << "Expected CONSTANT_Methodref, got " << cp_entry.tag
				  << "\n";
		context.exit("Bad constant pool entry");
	}

	auto name_and_type =
		context.classloader()->get_const_pool_entry(cp_entry.name_and_type_index);

	if (name_and_type.tag != ConstPoolTag::NameAndType) {
		std::cout << "Expected CONSTANT_NameAndType, got "
				  << name_and_type.tag << "\n";
		context.exit("Bad constant pool entry");
	}

	// FIXME utf8 isn't null terminated. sort of a big issue
	auto name_str =
		context.classloader()->get_const_pool_entry(name_and_type.name_index);
	std::cout << name_str.utf8 << "\n";

	// ugly
	context.set_stack_frame(&JVM::StackFrame::create(&context.stack_frame()));
	// TODO check if the method matches the type signature
	// otherwise the stack will be corrupted and we won't find the function
	// we're looking for.
	context.stack_frame().set_operating_bytecode(
		context.classloader()->methods[std::string(name_str.utf8)]);
}

void Opcodes::ICONST_M1(JVM &context) {
	context.operand_stack().push(-1);
}

void Opcodes::ICONST_0(JVM &context) {
	context.operand_stack().push(0);
}

void Opcodes::ICONST_1(JVM &context) {
	context.operand_stack().push(1);
}

void Opcodes::ICONST_2(JVM &context) {
	context.operand_stack().push(2);
}

void Opcodes::ICONST_3(JVM &context) {
	context.operand_stack().push(3);
}

void Opcodes::ICONST_4(JVM &context) {
	context.operand_stack().push(4);
}

void Opcodes::ICONST_5(JVM &context) {
	context.operand_stack().push(5);
}

void Opcodes::LCONST_0(JVM &context) {
	context.operand_stack().push_64(0L);
}

void Opcodes::LCONST_1(JVM &context) {
	context.operand_stack().push_64(1L);
}

void Opcodes::ISTORE_0(JVM &context) {
	context.istore(0, context.operand_stack().pop());
}

void Opcodes::ISTORE_1(JVM &context) {
	context.istore(1, context.operand_stack().pop());
}

void Opcodes::ISTORE_2(JVM &context) {
	context.istore(2, context.operand_stack().pop());
}

void Opcodes::ISTORE_3(JVM &context) {
	context.istore(3, context.operand_stack().pop());
}

void Opcodes::ILOAD_0(JVM &context) {
	context.operand_stack().push(
		context.stack_frame().local_variables[0]->get_fault_type<int>());
}

void Opcodes::ILOAD_1(JVM &context) {
	context.operand_stack().push(
		context.stack_frame().local_variables[1]->get_fault_type<int>());
}

void Opcodes::ILOAD_2(JVM &context) {
	context.operand_stack().push(
		context.stack_frame().local_variables[2]->get_fault_type<int>());
}

void Opcodes::LLOAD(JVM &context) {
	uint8_t index = context.opcode_parameters.at(0).get()->get_fault_type<int>();
	std::cout << (int)index << "\n";
	// FIXME bounds check
	context.operand_stack().push_64(
		context.stack_frame().local_variables[(uint8_t)index]->get_fault_type<int64_t>());
	std::cout << "LLOAD " << (int)index << "\n";
}

void Opcodes::IADD(JVM &context) {
	int32_t a = context.operand_stack().pop();
	int32_t b = context.operand_stack().pop();
	context.operand_stack().push(a + b);
	std::cout << "a + b = " << context.operand_stack().peek() << "\n";
}

void Opcodes::LADD(JVM &context) {
	int64_t a = context.operand_stack().pop_64();
	int64_t b = context.operand_stack().pop_64();
	std::cout << "a: " << a << " b: " << b << "\n";
	std::cout << "a + b = " << a + b << "\n";
	context.operand_stack().push_64(a + b);
	std::cout << "a + b = " << context.operand_stack().peek_64() << "\n";
}

void Opcodes::LDC2_W(JVM &context) {
	int index = context.opcode_parameters.at(0).get()->get_fault_type<int>();
	auto cp_entry = context.classloader()->get_const_pool_entry(index);
	if (cp_entry.tag != ConstPoolTag::Long) {
		std::cout << "Expected CONSTANT_Long, got " << cp_entry.tag << "\n";
		context.exit("Bad constant pool entry");
	}
	std::cout << "Pushing long " << cp_entry.numbers.long_integer << "\n";
	context.operand_stack().push_64(cp_entry.numbers.long_integer);
	std::cout << "Long pushed " << context.operand_stack().peek_64() << "\n";
}

void Opcodes::RETURN(JVM &context) {
	context.return_from_method();
}

void Opcodes::IRETURN(JVM &context) {
	auto parent = context.stack_frame().parent;
	parent->operand_stack.push(context.operand_stack().pop());
	context.return_from_method();
}

void Opcodes::BIPUSH(JVM &context) {
	int value = context.opcode_parameters.at(0).get()->get_fault_type<int>();
	context.operand_stack().push((uint8_t)value);
}

void Opcodes::IMUL(JVM &context) {
	int32_t a = context.operand_stack().pop();
	int32_t b = context.operand_stack().pop();
	context.operand_stack().push(a * b);
	std::cout << "a * b = " << context.operand_stack().peek() << "\n";
}

void Opcodes::IDIV(JVM &context) {
	int32_t a = context.operand_stack().pop();
	int32_t b = context.operand_stack().pop();
	context.operand_stack().push(a / b);
	std::cout << "a / b = " << context.operand_stack().peek() << "\n";
}

void Opcodes::LMUL(JVM &context) {
	int64_t a = context.operand_stack().pop_64();
	int64_t b = context.operand_stack().pop_64();
	context.operand_stack().push_64(a * b);
}

void Opcodes::ISTORE(JVM &context) {
	int index = context.opcode_parameters.at(0).get()->get_fault_type<int>();
	context.istore((uint8_t)index, context.operand_stack().pop());
}

void Opcodes::LSTORE(JVM &context) {
	int index = context.opcode_parameters.at(0).get()->get_fault_type<int>();
	context.lstore((uint8_t)index, context.operand_stack().pop_64());
}

void Opcodes::NEWARRAY(JVM &context) {
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
	uint8_t primitive_index = (uint8_t)context.opcode_parameters.at(0).get()->get_fault_type<int>();
	uint32_t array_size = context.operand_stack().pop();
	std::cout << "primtiive_index " << (int)primitive_index << "\n";
	std::cout << "array_size " << (int)array_size << "\n";
	context.exit("TODO NEWARRAY", 1);
}

void Opcodes::Unknown(JVM& context) {
	std::cout << "Unknown opcode\n";
	std::cout << "Exiting...\n";
	context.exit("Bad opcode", 1);
}

std::map<uint8_t, OpcodeHandle> Opcodes::opcode_map = {
	{0x00, OpcodeHandle{.no_parameters = 0, .function = Opcodes::NOP}},
	{0x02, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_M1}},

	{0x03, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_0}},
	{0x04, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_1}},
	{0x05, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_2}},
	{0x06, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_3}},
	{0x07, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_4}},
	{0x08, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ICONST_5}},

	{0x09, OpcodeHandle{.no_parameters = 0, .function = Opcodes::LCONST_0}},
	{0x0a, OpcodeHandle{.no_parameters = 0, .function = Opcodes::LCONST_1}},

	{0x10, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Byte, .function = Opcodes::BIPUSH}},

	{0x14, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Short, .function = Opcodes::LDC2_W}},

	{0x16, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Byte, .function = Opcodes::LLOAD}},

	{0x36, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Byte, .function = Opcodes::ISTORE}},
	{0x37, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Byte, .function = Opcodes::LSTORE}},


	{0x3b, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ISTORE_0}},
	{0x3c, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ISTORE_1}},
	{0x3d, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ISTORE_2}},
	{0x3e, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ISTORE_3}},


	{0x1a, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ILOAD_0}},
	{0x1b, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ILOAD_1}},
	{0x1c, OpcodeHandle{.no_parameters = 0, .function = Opcodes::ILOAD_2}},

	{0x60, OpcodeHandle{.no_parameters = 0, .function = Opcodes::IADD}},
	{0x61, OpcodeHandle{.no_parameters = 0, .function = Opcodes::LADD}},
	{0x68, OpcodeHandle{.no_parameters = 0, .function = Opcodes::IMUL}},
	{0x69, OpcodeHandle{.no_parameters = 0, .function = Opcodes::LMUL}},
	{0x6c, OpcodeHandle{.no_parameters = 0, .function = Opcodes::IDIV}},

	{0xb1, OpcodeHandle{.no_parameters = 0, .function = Opcodes::RETURN}},
	{0xac, OpcodeHandle{.no_parameters = 0, .function = Opcodes::IRETURN}},

	{0xb8, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Short, .function = Opcodes::INVOKESTATIC}},

	{0xbc, OpcodeHandle{.no_parameters = 1, .parameter_type = OpcodeHandle::Byte, .function = Opcodes::NEWARRAY}},
};

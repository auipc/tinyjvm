#pragma once
#include <cstdint>
#include <map>

class JVM;

typedef void (*OpcodeFunc)(JVM& context);
struct OpcodeHandle {
	uint32_t no_parameters;
	enum {
		Byte,
		Short,
		Int,
		Long
	} parameter_type;
	OpcodeFunc function;
};

namespace Opcodes {
	void NOP(JVM& context);
	void INVOKESTATIC(JVM& context);

	void BIPUSH(JVM& context);

	void ICONST_M1(JVM& context);
	void ICONST_0(JVM& context);
	void ICONST_1(JVM& context);
	void ICONST_2(JVM& context);
	void ICONST_3(JVM& context);
	void ICONST_4(JVM& context);
	void ICONST_5(JVM& context);

	void LCONST_0(JVM& context);
	void LCONST_1(JVM& context);

	void ISTORE_0(JVM& context);
	void ISTORE_1(JVM& context);
	void ISTORE_2(JVM& context);
	void ISTORE_3(JVM& context);

	void ISTORE(JVM& context);
	void LSTORE(JVM& context);

	void LLOAD(JVM& context);

	void IADD(JVM& context);
	void LADD(JVM& context);
	void LMUL(JVM& context);

	void ILOAD_0(JVM& context);
	void ILOAD_1(JVM& context);
	void ILOAD_2(JVM& context);

	void IMUL(JVM& context);
	void IDIV(JVM& context);

	void LDC2_W(JVM& context);

	void RETURN(JVM& context);
	void IRETURN(JVM& context);

	void NEWARRAY(JVM& context);

	void Unknown(JVM& context);

	extern std::map<uint8_t, OpcodeHandle> opcode_map;
};

#pragma once
#include <cstdint>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

#define CLASS_MAGIC 0xCAFEBABE

class ClassFileStream {
  public:
	ClassFileStream(const char *filename);
	~ClassFileStream();
	std::vector<char> consume(long long bytes);
	template <typename T> T read();
	void read_length(void *buffer, size_t length);
	uint32_t read_u4();
	uint16_t read_u2();

	inline long long tell() { return m_position; }

  private:
	long long m_length = 0;
	long long m_position = 0;
	char *m_buffer = nullptr;
	const char *m_filename;
	FILE *m_file;
};

enum ConstPoolTag {
	Null = 0,
	Utf8 = 1,
	Integer = 3,
	Float = 4,
	Long = 5,
	Double = 6,
	Class = 7,
	String = 8,
	FieldRef = 9,
	MethodRef = 10,
	NameAndType = 12,
};

inline const char *constant_pool_enum_name(ConstPoolTag tag) {
	switch (tag) {
	case ConstPoolTag::Null:
		return "Null";
	case ConstPoolTag::Utf8:
		return "Utf8";
	case ConstPoolTag::Integer:
		return "Integer";
	case ConstPoolTag::Float:
		return "Float";
	case ConstPoolTag::Long:
		return "Long";
	case ConstPoolTag::Double:
		return "Double";
	case ConstPoolTag::Class:
		return "Class";
	case ConstPoolTag::String:
		return "String";
	case ConstPoolTag::FieldRef:
		return "FieldRef";
	case ConstPoolTag::MethodRef:
		return "MethodRef";
	case ConstPoolTag::NameAndType:
		return "NameAndType";
	default:
		return "Unknown";
	}
}

// Stupid design using a single mega structure with null types
// This is ideally more secure as a mixed size array would have to keep sizes
// that might not be valid but still
struct ConstPoolEntry {
	ConstPoolTag tag = ConstPoolTag::Null;
	char *utf8 = nullptr;
	size_t utf8_length = 0;
	union {
		int32_t integer;
		int64_t long_integer;
		float float_number;
		double double_number;
	} numbers;

	uint16_t class_index;
	uint16_t name_and_type_index;
	uint16_t name_index;
};

struct Method {
	uint16_t max_stack;
	uint16_t max_locals;
	uint32_t code_length;
	uint8_t *code;
	uint16_t class_index;
};

class ClassLoader {
  public:
	ClassLoader(std::string class_name, std::string filename);
	~ClassLoader();
	void load_class();
	std::map<std::string, Method> methods;
	ConstPoolEntry get_const_pool_entry(uint16_t index) {
		return constant_pool.at(index - 1);
	}

	std::map<uint16_t, std::string> classes;
  private:
	void read_attributes(char *utf8, size_t utf8_length, uint16_t count);
	void read(char *buffer, size_t size);
	std::vector<ConstPoolEntry> constant_pool;
	// void read_const_pool(ClassFile_1& cfile);
	ClassFileStream *m_stream = nullptr;
	std::string m_filename;
	std::string m_class_name;
};

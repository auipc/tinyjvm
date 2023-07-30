#pragma once
#include <vector>
#include <stdio.h>
#include <cstdint>
#include <map>
#include <string>

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
	Class = 7,
	String = 8,
	FieldRef = 9,
	MethodRef = 10,
	NameAndType = 12,
};

// Stupid design using a single mega structure with null types
// This is ideally more secure as a mixed size array would have to keep sizes
// that might not be valid but still
struct ConstPoolEntry {
	ConstPoolTag tag = ConstPoolTag::Null;
	char *utf8 = nullptr;
	size_t utf8_length = 0;
};

struct MethodCode {
	uint16_t max_stack;
	uint16_t max_locals;
	uint32_t code_length;
	uint8_t *code;
};

class ClassLoader {
  public:
	ClassLoader(std::string filename);
	~ClassLoader();
	void load_class();
	std::map<std::string, MethodCode> method_code;
  private:
	void read_attributes(char *utf8, size_t utf8_length, uint16_t count);
	void read(char *buffer, size_t size);
	std::vector<ConstPoolEntry> constant_pool;
	// void read_const_pool(ClassFile_1& cfile);
	ClassFileStream *m_stream = nullptr;
	std::string m_filename;
};

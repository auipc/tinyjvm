#pragma once
#include <stdint.h>
#include <exception>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>
#include <tinyjvm/ConstantPool.h>
#include <assert.h>

#define CLASS_MAGIC 0xCAFEBABE

namespace tinyJVM {
class ClassParseException : public std::exception {
  public:
	ClassParseException(const char *message) : m_message(message) {}
	const char *what() { return m_message; }

  private:
	const char *m_message;
};

class ClassFileStream {
  public:
	ClassFileStream(const char *filename);
	~ClassFileStream();
	std::vector<char> consume(long long bytes);
	template <typename T> T read();
	void read_length(void *buffer, size_t length);

	inline long long tell() { return m_position; }

  private:
	long long m_length = 0;
	long long m_position = 0;
	char *m_buffer = nullptr;
	const char *m_filename;
	FILE *m_file;
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

	inline ConstantPool& constant_pool() const {
		assert(m_constant_pool);
		return *m_constant_pool;
	}

	std::map<uint16_t, std::string> classes;
  private:
	void read_attributes(char *utf8, size_t utf8_length, uint16_t count);
	void read(char *buffer, size_t size) const;
	ConstantPool* m_constant_pool;
	ClassFileStream *m_stream = nullptr;
	std::string m_filename;
	std::string m_class_name;
};
}
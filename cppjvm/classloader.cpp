#include <cppjvm/classloader.h>
#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cppjvm/exceptions.h>

ClassFileStream::ClassFileStream(const char *filename) : m_filename(filename) {
	// I get le unix mindset is not having bloat
	// but why isn't there a flag to read the entire file?
	// it saves syscalls giving more time to userspace
	// better 4 performance
	// I think FILE in the C std has the length but nobody cares about that

	m_file = fopen(m_filename, "r");
	if (!m_file) {
		throw ErrnoException();
	}

	// These can fail
	// FIXME handle these
	fseek(m_file, 0, SEEK_END);
	m_length = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);

	m_buffer = new char[m_length];
	memset(m_buffer, 0, m_length);
	printf("lol 0x%x\n", m_buffer);
	fread(m_buffer, m_length, 1, m_file);
	for (int i = 0; i < m_length; i++) {
		printf("0x%x ", m_buffer[i] & 0xFF);
	}
	printf("\n");
	// fclose(m_file);
}

ClassFileStream::~ClassFileStream() {
	m_position = 0;
	delete[] m_buffer;
}

std::vector<char> ClassFileStream::consume(long long bytes) {
	std::vector<char> data;
	if ((m_position + bytes) >= m_length) {
		throw std::runtime_error("EOF");
	}

	data.reserve(bytes);
	memcpy(data.data(), m_buffer + m_position, bytes);
	m_position += bytes;

	return data;
}

template <typename T> void SwapEndian(T &val) {
	union U {
		T val;
		std::array<std::uint8_t, sizeof(T)> raw;
	} src, dst;

	src.val = val;
	std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
	val = dst.val;
}

template <typename T> T ClassFileStream::read() {
	T x;
	std::vector<char> data = consume(sizeof(x));
	memcpy(&x, data.data(), sizeof(x));
	SwapEndian(x);
	return x;
}

void ClassFileStream::read_length(void *buffer, size_t length) {
	std::vector<char> data = consume(sizeof(uint8_t) * length);
	memcpy(buffer, data.data(), sizeof(uint8_t) * length);
	SwapEndian(buffer);
}

uint32_t ClassFileStream::read_u4() {
	uint32_t u4;
	std::cout << "pos " << m_position << "\n";
	std::vector<char> data = consume(4);
	memcpy(&u4, data.data(), 4);
	u4 = htonl(u4);

	return u4;
}

uint16_t ClassFileStream::read_u2() {
	uint16_t u2;
	std::vector<char> data = consume(2);
	memcpy(&u2, data.data(), 2);
	u2 = htons(u2);
	m_position += 2;

	return u2;
}

ClassLoader::ClassLoader(std::string filename)
	: m_filename(filename)
{}

ClassLoader::~ClassLoader() {}

void ClassLoader::read_attributes(char *utf8, size_t utf8_length,
								  uint16_t count) {
	for (int i = 0; i < count; i++) {
		uint16_t attribute_name_idx = m_stream->read<uint16_t>();
		uint32_t attribute_length = m_stream->read<uint32_t>();
		std::cout << "idx m " << attribute_name_idx - 1 << "\n";
		auto cp_entry = constant_pool.at(attribute_name_idx - 1);
		// uint8_t* info = new uint8_t[attribute_length];
		// m_stream->read_length(reinterpret_cast<void*>(info),
		// attribute_length);
		if (strncmp(cp_entry.utf8, "Code", cp_entry.utf8_length) == 0) {
			printf("Code attribute found\n");
			uint16_t max_stack = m_stream->read<uint16_t>();
			uint16_t max_locals = m_stream->read<uint16_t>();
			uint32_t code_length = m_stream->read<uint32_t>();
			uint8_t *code = new uint8_t[code_length];
			m_stream->read_length(reinterpret_cast<void *>(code), code_length);
			for (int o = 0; o < code_length; o++) {
				printf("%x ", code[o]);
			}
			printf("\n");

			method_code[std::string(utf8, utf8_length)] =
				MethodCode{max_stack, max_locals, code_length, code};
			std::cout << "lel: " << std::string(utf8, utf8_length) << "\n";

			uint16_t exception_table_length = m_stream->read<uint16_t>();
			if (exception_table_length > 0) {
				throw std::runtime_error(
					"exception_table_length > 0. we don't know how to parse "
					"exception_table_length!");
			}

			uint16_t attributes_count = m_stream->read<uint16_t>();
			read_attributes(utf8, utf8_length, attributes_count);
		} else if (strncmp(cp_entry.utf8, "LineNumberTable",
						   cp_entry.utf8_length) == 0) {
			uint16_t line_number_table_length = m_stream->read<uint16_t>();
			for (int j = 0; j < line_number_table_length; j++) {
				uint16_t start_pc = m_stream->read<uint16_t>();
				uint16_t line_number = m_stream->read<uint16_t>();
			}
		} else if (strncmp(cp_entry.utf8, "ConstantValue",
						   cp_entry.utf8_length) == 0) {
			uint16_t constantvalue_index = m_stream->read<uint16_t>();
		} else if (strncmp(cp_entry.utf8, "StackMapTable",
						   cp_entry.utf8_length) == 0) {
			uint16_t number_of_entries = m_stream->read<uint16_t>();
			for (int j = 0; j < number_of_entries; j++) {
				uint8_t frame_type = m_stream->read<uint8_t>();
				if (frame_type >= 0 && frame_type <= 63) {
					// same_frame
					throw std::runtime_error("same_frame not implemented");
				} else if (frame_type >= 64 && frame_type <= 127) {
					// same_locals_1_stack_item_frame
					throw std::runtime_error(
						"same_locals_1_stack_item_frame not implemented");
				} else if (frame_type == 247) {
					// same_locals_1_stack_item_frame_extended
					throw std::runtime_error("same_locals_1_stack_item_frame_"
											 "extended not implemented");
				} else if (frame_type >= 248 && frame_type <= 250) {
					// chop_frame
					uint16_t offset_delta = m_stream->read<uint16_t>();
				} else if (frame_type == 251) {
					// same_frame_extended
					throw std::runtime_error(
						"same_frame_extended not implemented");
				} else if (frame_type >= 252 && frame_type <= 254) {
					// append_frame
					throw std::runtime_error("append_frame not implemented");
				} else if (frame_type == 255) {
					// full_frame
					uint16_t offset_delta = m_stream->read<uint16_t>();
					uint16_t number_of_locals = m_stream->read<uint16_t>();
					// read verification_type_info
					for (int i = 0; i < number_of_locals; i++) {
						uint8_t tag = m_stream->read<uint8_t>();
						std::cout << "tag: " << (int)tag << "\n";
						if (tag == 7) {
							uint16_t cpool_index = m_stream->read<uint16_t>();
							std::cout << "cpool_index: " << cpool_index << "\n";
						} else if (tag == 8) {
							uint16_t offset = m_stream->read<uint16_t>();
							std::cout << "offset: " << offset << "\n";
						} else if (tag == 6) {
							throw std::runtime_error("Unimplemented tag");
						} else if (tag == 4) {
							throw std::runtime_error("Unimplemented tag");
						} else if (tag == 3) {
							throw std::runtime_error("Unimplemented tag");
						} else if (tag == 2) {
							throw std::runtime_error("Unimplemented tag");
						} else if (tag == 1) {
							// throw std::runtime_error("Unimplemented tag");
						} else if (tag == 0) {
							throw std::runtime_error("Unimplemented tag");
						} else if (tag == 5) {
							throw std::runtime_error("Unimplemented tag");
						}
					}

					uint16_t number_of_stack_items = m_stream->read<uint16_t>();
					for (int i = 0; i < number_of_stack_items; i++) {
						throw std::runtime_error("Unknown tag");
					}
				}
			}
		} else {
			printf("str: %.*s\n", cp_entry.utf8_length, cp_entry.utf8);
			throw std::runtime_error("Unknown attribute");
		}
		printf("str: %.*s\n", cp_entry.utf8_length, cp_entry.utf8);
	}
}

// TODO methods
void ClassLoader::load_class() {
	m_stream = new ClassFileStream(m_filename.c_str());

	uint32_t magic = m_stream->read<uint32_t>();
	if (magic != CLASS_MAGIC) {
		throw std::runtime_error("Magic number missing");
	}

	uint16_t minor = m_stream->read<uint16_t>();
	uint16_t major = m_stream->read<uint16_t>();
	std::cout << "m " << minor << "\n";
	std::cout << "m " << major << "\n";

	uint16_t constant_pool_size = m_stream->read<uint16_t>();
	for (int i = 0; i < constant_pool_size - 1; i++) {
		uint8_t tag = m_stream->read<uint8_t>();
		// TODO parse each object in the pool
		switch (tag) {
		case ConstPoolTag::Utf8: {
			uint16_t length = m_stream->read<uint16_t>();
			// This isn't terminated by a NUL byte so be careful when operating
			// on it.
			char *buffer = new char[length];
			m_stream->read_length(reinterpret_cast<void *>(buffer), length);
			constant_pool.push_back(
				ConstPoolEntry{ConstPoolTag::Utf8, buffer, length});
			// FIXME delete buffer for now
			// delete[] buffer;
		} break;
		case ConstPoolTag::String: {
			uint16_t string_index = m_stream->read<uint16_t>();
			constant_pool.push_back(ConstPoolEntry{ConstPoolTag::String});
		} break;
		case ConstPoolTag::Class: {
			uint16_t name_index = m_stream->read<uint16_t>();
			constant_pool.push_back(ConstPoolEntry{ConstPoolTag::Class});
		} break;
		case ConstPoolTag::FieldRef: {
			uint16_t class_idx = m_stream->read<uint16_t>();
			uint16_t name_and_type_idx = m_stream->read<uint16_t>();
			constant_pool.push_back(ConstPoolEntry{ConstPoolTag::FieldRef});
		} break;
		case ConstPoolTag::MethodRef: {
			uint16_t class_idx = m_stream->read<uint16_t>();
			uint16_t name_and_type_idx = m_stream->read<uint16_t>();
			constant_pool.push_back(ConstPoolEntry{ConstPoolTag::MethodRef});
		} break;
		case ConstPoolTag::NameAndType: {
			uint16_t name_idx = m_stream->read<uint16_t>();
			uint16_t descriptor_idx = m_stream->read<uint16_t>();
			constant_pool.push_back(ConstPoolEntry{ConstPoolTag::NameAndType});
		} break;
		default:
			throw std::runtime_error("Unknown constant pool tag");
			break;
		}
		// break;
	}

	uint16_t access_flags = m_stream->read<uint16_t>();
	uint16_t this_class = m_stream->read<uint16_t>();
	uint16_t super_class = m_stream->read<uint16_t>();
	uint16_t interfaces_count = m_stream->read<uint16_t>();

	// interfaces unused
	if (interfaces_count > 0) {
		uint16_t *interfaces = new uint16_t[interfaces_count];
		m_stream->read_length(reinterpret_cast<void *>(interfaces),
							  interfaces_count * sizeof(uint16_t));
		delete[] interfaces;
	}

	uint16_t fields_count = m_stream->read<uint16_t>();
	std::cout << "fields_count: " << fields_count << "\n";
	if (fields_count > 0) {
		for (int i = 0; i < fields_count; i++) {
			uint16_t access_flags = m_stream->read<uint16_t>();
			uint16_t name_index = m_stream->read<uint16_t>();
			uint16_t descriptor_index = m_stream->read<uint16_t>();
			uint16_t attributes_count = m_stream->read<uint16_t>();
			if (attributes_count > 0) {
				auto cp_entry = constant_pool.at(name_index - 1);
				read_attributes(cp_entry.utf8, cp_entry.utf8_length,
								attributes_count);
			}
		}
	}

	uint16_t methods_count = m_stream->read<uint16_t>();
	if (methods_count > 0) {
		for (int i = 0; i < methods_count; i++) {
			uint16_t access_flags = m_stream->read<uint16_t>();
			uint16_t name_index = m_stream->read<uint16_t>();
			std::cout << "name_index " << name_index << "\n";
			auto cp_entry = constant_pool.at(name_index - 1);
			uint16_t descriptor_index = m_stream->read<uint16_t>();
			uint16_t attributes_count = m_stream->read<uint16_t>();
			read_attributes(cp_entry.utf8, cp_entry.utf8_length,
							attributes_count);
		}
	}

	printf("\033[0;31mFIXME: buggy stack map table parsing so we can't read "
		   "attribute count\033[0m\n");
	// uint16_t attributes_count = m_stream->read<uint16_t>();
	// read_attributes("", 0, attributes_count);

	delete m_stream;
}

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <tinyjvm/ClassLoader.h>
#include <tinyjvm/Exceptions.h>

namespace tinyJVM {

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
	fread(m_buffer, m_length, 1, m_file);
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

void ClassFileStream::read_length(void *buffer, size_t length) {
	std::vector<char> data = consume(sizeof(uint8_t) * length);
	memcpy(buffer, data.data(), sizeof(uint8_t) * length);
	SwapEndian(buffer);
}

template <typename T> T ClassFileStream::read() {
	T x;
	std::vector<char> data = consume(sizeof(x));
	memcpy(&x, data.data(), sizeof(x));
	SwapEndian(x);
	return x;
}

ClassLoader::ClassLoader(std::string class_name, std::string filename)
	: m_class_name(class_name), m_filename(filename), m_constant_pool(nullptr) {}

ClassLoader::~ClassLoader() {}

void ClassLoader::read_attributes(char *utf8, size_t utf8_length,
								  uint16_t count) {
	for (int i = 0; i < count; i++) {
		uint16_t attribute_name_idx = m_stream->read<uint16_t>();
		uint32_t attribute_length = m_stream->read<uint32_t>();
		auto cp_entry = constant_pool().get(attribute_name_idx); //.at(attribute_name_idx - 1);

		// Skip over all entries not of type Utf8
		if (cp_entry.tag != ConstantPool::PoolEntry::Tag::Utf8)
			continue;
		// uint8_t* info = new uint8_t[attribute_length];
		// m_stream->read_length(reinterpret_cast<void*>(info),
		// attribute_length);
		if (strncmp(cp_entry.utf8.string, "Code", cp_entry.utf8.length) == 0) {
			uint16_t max_stack = m_stream->read<uint16_t>();
			uint16_t max_locals = m_stream->read<uint16_t>();
			uint32_t code_length = m_stream->read<uint32_t>();
			uint8_t *code = new uint8_t[code_length];
			m_stream->read_length(reinterpret_cast<void *>(code), code_length);

			methods[std::string(utf8, utf8_length)] =
				Method{max_stack, static_cast<uint16_t>(max_locals-1), code_length, code};

			uint16_t exception_table_length = m_stream->read<uint16_t>();
			if (exception_table_length > 0) {
				throw ClassParseException(
					"exception_table_length > 0. we don't know how to parse "
					"exception_table_length!");
			}

			uint16_t attributes_count = m_stream->read<uint16_t>();
			read_attributes(utf8, utf8_length, attributes_count);
		} else if (strncmp(cp_entry.utf8.string, "LineNumberTable",
						   cp_entry.utf8.length) == 0) {
			uint16_t line_number_table_length = m_stream->read<uint16_t>();
			for (int j = 0; j < line_number_table_length; j++) {
				uint16_t start_pc = m_stream->read<uint16_t>();
				uint16_t line_number = m_stream->read<uint16_t>();
			}
		} else if (strncmp(cp_entry.utf8.string, "ConstantValue",
						   cp_entry.utf8.length) == 0) {
			uint16_t constantvalue_index = m_stream->read<uint16_t>();
		} else if (strncmp(cp_entry.utf8.string, "StackMapTable",
						   cp_entry.utf8.length) == 0) {
			uint16_t number_of_entries = m_stream->read<uint16_t>();
			for (int j = 0; j < number_of_entries; j++) {
				uint8_t frame_type = m_stream->read<uint8_t>();
				if (frame_type >= 0 && frame_type <= 63) {
					// same_frame
					throw ClassParseException("same_frame not implemented");
				} else if (frame_type >= 64 && frame_type <= 127) {
					// same_locals_1_stack_item_frame
					throw ClassParseException(
						"same_locals_1_stack_item_frame not implemented");
				} else if (frame_type == 247) {
					// same_locals_1_stack_item_frame_extended
					throw ClassParseException("same_locals_1_stack_item_frame_"
											  "extended not implemented");
				} else if (frame_type >= 248 && frame_type <= 250) {
					// chop_frame
					uint16_t offset_delta = m_stream->read<uint16_t>();
				} else if (frame_type == 251) {
					// same_frame_extended
					throw ClassParseException(
						"same_frame_extended not implemented");
				} else if (frame_type >= 252 && frame_type <= 254) {
					// append_frame
					uint16_t offset_delta = m_stream->read<uint16_t>();
					uint16_t number_of_locals = frame_type - 251;
					// read verification_type_info
					for (int i = 0; i < number_of_locals; i++) {
						uint8_t tag = m_stream->read<uint8_t>();
						switch (tag) {
						case 1:
							break;
						case 4:
							break;
						case 7: {
							uint16_t cpool_index = m_stream->read<uint16_t>();
						} break;
						case 8: {
							uint16_t offset = m_stream->read<uint16_t>();
						} break;
						default:
							throw ClassParseException("unknown tag");
							break;
						}
					}
				} else if (frame_type == 255) {
					// full_frame
					uint16_t offset_delta = m_stream->read<uint16_t>();
					uint16_t number_of_locals = m_stream->read<uint16_t>();
					// read verification_type_info
					for (int i = 0; i < number_of_locals; i++) {
						uint8_t tag = m_stream->read<uint8_t>();
						switch (tag) {
							case 7:
								(void)m_stream->read<uint16_t>();
								break;
							case 8:
								(void)m_stream->read<uint16_t>();
								break;
							// These can just be ignored, for now.
							case 1:
							case 4:
								break;
							default:
								throw ClassParseException("Unimplemented tag");
								break;
						}
					}

					uint16_t number_of_stack_items = m_stream->read<uint16_t>();
					for (int i = 0; i < number_of_stack_items; i++) {
						throw ClassParseException("Unknown tag");
					}
				}
			}
		} else {
			throw ClassParseException("Unknown attribute");
		}
	}
}

void ClassLoader::load_class() {
	m_stream = new ClassFileStream(m_filename.c_str());

	uint32_t magic = m_stream->read<uint32_t>();
	if (magic != CLASS_MAGIC) {
		throw ClassParseException("Incompatible magic number");
	}

	uint16_t minor = m_stream->read<uint16_t>();
	uint16_t major = m_stream->read<uint16_t>();

	uint16_t constant_pool_size = m_stream->read<uint16_t>();

	// Create a constant pool container
	m_constant_pool = new ConstantPool(constant_pool_size);

	for (int i = 0; i < constant_pool_size - 1; i++) {
		uint8_t tag = m_stream->read<uint8_t>();
		switch (tag) {
		case ConstantPool::PoolEntry::Tag::Utf8: {
			uint16_t length = m_stream->read<uint16_t>();
			// This isn't terminated by a NUL byte so be careful when operating
			// on it.
			char *buffer = new char[length];
			m_stream->read_length(reinterpret_cast<void *>(buffer), length);
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::Utf8,
				.utf8 = {
					.string = buffer,
					.length = length
				}
			});
			//constant_pool.push_back(
				//ConstPoolEntry{ConstantPool::PoolEntry::Tag::Utf8, buffer, length});
			// FIXME delete buffer for now
			// delete[] buffer;
		} break;
		case ConstantPool::PoolEntry::Tag::Long: {
			int64_t value = m_stream->read<int64_t>();
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::Long,
				.number = {
					.long_integer = value
				}
			});
			// All 8-byte constants take up two entries in the constant_pool
			// table of the class file. If a CONSTANT_Long_info or
			// CONSTANT_Double_info structure is the entry at index n in the
			// constant_pool table, then the next usable entry in the table is
			// located at index n+2. The constant_pool index n+1 must be valid
			// but is considered unusable.
			i++;
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::Null
			});
		} break;
		case ConstantPool::PoolEntry::Tag::String: {
			uint16_t string_index = m_stream->read<uint16_t>();
			//constant_pool.push_back(ConstPoolEntry{ConstantPool::PoolEntry::Tag::String});
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::String,
			});
		} break;
		case ConstantPool::PoolEntry::Tag::Class: {
			uint16_t name_idx = m_stream->read<uint16_t>();
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::Class,
				.class_ref = {
					.name_index = name_idx
				}
			});
		} break;
		case ConstantPool::PoolEntry::Tag::FieldRef: {
			uint16_t class_idx = m_stream->read<uint16_t>();
			uint16_t name_and_type_idx = m_stream->read<uint16_t>();
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::FieldRef,
			});
		} break;
		case ConstantPool::PoolEntry::Tag::MethodRef: {
			uint16_t class_idx = m_stream->read<uint16_t>();
			uint16_t name_and_type_idx = m_stream->read<uint16_t>();
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::MethodRef,
				.method_ref = {
					.class_index = class_idx,
					.name_and_type_index = name_and_type_idx
				}
			});
		} break;
		case ConstantPool::PoolEntry::Tag::NameAndType: {
			uint16_t name_idx = m_stream->read<uint16_t>();
			uint16_t descriptor_idx = m_stream->read<uint16_t>();
			m_constant_pool->add_entry(i, ConstantPool::PoolEntry{
				.tag = ConstantPool::PoolEntry::Tag::NameAndType,
				.name_and_type = {
					.name_index = name_idx,
					.descriptor_index = descriptor_idx
				}
			});
		} break;
		default:
			throw ClassParseException("Unknown constant pool tag");
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
	if (fields_count > 0) {
		for (int i = 0; i < fields_count; i++) {
			uint16_t access_flags = m_stream->read<uint16_t>();
			uint16_t name_index = m_stream->read<uint16_t>();
			uint16_t descriptor_index = m_stream->read<uint16_t>();
			uint16_t attributes_count = m_stream->read<uint16_t>();
			if (attributes_count > 0) {
				auto cp_entry = constant_pool().get(name_index);
				read_attributes(cp_entry.utf8.string, cp_entry.utf8.length,
								attributes_count);
			}
		}
	}

	uint16_t methods_count = m_stream->read<uint16_t>();
	if (methods_count > 0) {
		for (int i = 0; i < methods_count; i++) {
			uint16_t access_flags = m_stream->read<uint16_t>();
			uint16_t name_index = m_stream->read<uint16_t>();
			auto cp_entry = constant_pool().get(name_index);
			uint16_t descriptor_index = m_stream->read<uint16_t>();
			uint16_t attributes_count = m_stream->read<uint16_t>();
			read_attributes(cp_entry.utf8.string, cp_entry.utf8.length,
							attributes_count);
		}
	}

	// Iterate through constant pool method refs and match them with classes
	std::vector<std::string> method_names;
	for (int i = 0; i < constant_pool().size(); i++) {
		auto entry = constant_pool().get(i);
		// Resolve main method's class index
		if (entry.tag == ConstantPool::PoolEntry::Tag::Class) {
			auto name = constant_pool().get(entry.class_ref.name_index);
			if (strncmp(name.utf8.string, m_class_name.c_str(), name.utf8.length) ==
				0) {
				methods["main"].class_index = i + 1;
			}
			classes[i] = std::string(name.utf8.string, name.utf8.length);
		}

		if (entry.tag == ConstantPool::PoolEntry::Tag::MethodRef) {
			auto name = constant_pool().get(entry.method_ref.class_index);
			auto name_and_type =
				constant_pool().get(entry.method_ref.name_and_type_index);
			auto name_str = constant_pool().get(name_and_type.name_and_type.name_index);
			methods[name_str.utf8.string].class_index = entry.method_ref.class_index;
		}
	}

	delete m_stream;
}
}
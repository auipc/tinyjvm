#pragma once
#include <array>
#include <stdint.h>
#include <stddef.h>

class ConstantPool {
public:
    ConstantPool(size_t size);
    ~ConstantPool();

    struct Utf8String {
        // TODO Free this eventually
        char* string;
        uint16_t length;
    };

    struct Number {
        union {
            int32_t integer;
            int64_t long_integer;
            float float_number;
            double double_number;
        };
    };

    struct Class {
        uint16_t name_index;
    };

    struct MethodRef {
        uint16_t class_index;
        uint16_t name_and_type_index;
    };

    struct NameAndType {
        uint16_t name_index;
        uint16_t descriptor_index;
    };

    // Megastruct that encompasses all pool entries.
    // I can't think of something else that doesn't involve a bunch of annoying code
    // that keeps track of what we store and the sizes of them so this is what we're getting.
    struct PoolEntry {
        enum Tag {
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

        Tag tag = Tag::Null;
        union {
            ConstantPool::Utf8String utf8;
            ConstantPool::Number number;
            ConstantPool::Class class_ref;
            ConstantPool::MethodRef method_ref;
            ConstantPool::NameAndType name_and_type;
        };
    };

    const PoolEntry& get(uint16_t index) const;
    void add_entry(uint16_t index, PoolEntry entry);
    inline size_t size() const { return m_pool_entries_size; }
private:
    PoolEntry* m_pool_entries;
    size_t m_pool_entries_size;
};
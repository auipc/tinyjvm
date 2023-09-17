#include <string.h>
#include <stdexcept>

#include <tinyjvm/ConstantPool.h>

namespace tinyJVM {
ConstantPool::ConstantPool(size_t size)
    : m_pool_entries_size(size)
{
    m_pool_entries = new PoolEntry[size];
    memset(m_pool_entries, 0, sizeof(m_pool_entries));
}

ConstantPool::~ConstantPool()
{
    delete[] m_pool_entries;
    m_pool_entries_size = 0;
}

void ConstantPool::add_entry(uint16_t index, PoolEntry entry) {
    m_pool_entries[index] = entry;
}

const ConstantPool::PoolEntry& ConstantPool::get(uint16_t index) const {
    return m_pool_entries[index-1];
}
}
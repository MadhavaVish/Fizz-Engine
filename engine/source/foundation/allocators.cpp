#include <foundation/allocators.hpp>
#include <stdlib.h>
#include <windows.h>

namespace fizzengine {
sizet memory_align(sizet size, sizet alignment) {
    const sizet alignment_mask = alignment - 1;
    return (size + alignment_mask) & ~alignment_mask;
}

void* HeapAllocator::allocate(sizet size, sizet alignment) {
    return malloc(size);
}

void HeapAllocator::deallocate(void* pointer) {
    free(pointer);
}

bool Arena::init(size_t size) {
    if (base != nullptr)
        return false; // Already initialized

    base = (u8*)VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
    if (!base) {
        return false;
    }

    allocated_size = 0;
    committed_size = 0;
    total_size     = size;
    return true;
}

void Arena::shutdown() {
    if (base == nullptr)
        return; // Not initialized

    VirtualFree(base, 0, MEM_RELEASE);

    base           = nullptr;
    allocated_size = 0;
    committed_size = 0;
    total_size     = 0;
}

void* Arena::allocate(size_t size, size_t alignment) {
    if (base == nullptr)
        return nullptr; // Not initialized

    const sizet new_start          = memory_align(allocated_size, alignment);
    const sizet new_allocated_size = new_start + size;
    if (new_allocated_size > total_size)
        return nullptr;

    if (new_allocated_size > committed_size) {

        sizet bytes_needed    = new_allocated_size - committed_size;
        sizet pages_needed    = (bytes_needed + PAGE_SIZE - 1) / PAGE_SIZE;
        sizet bytes_to_commit = pages_needed * PAGE_SIZE;

        if (!VirtualAlloc(base + committed_size, bytes_to_commit, MEM_COMMIT, PAGE_READWRITE)) {
            return nullptr;
        }
        committed_size += bytes_to_commit;
    }

    allocated_size = new_allocated_size;
    return base + new_start;
}

void Arena::deallocate(void* pointer) {
    // No-op for arena allocator
}

} // namespace fizzengine
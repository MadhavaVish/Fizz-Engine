#pragma once

#include <foundation/platform.hpp>

namespace fizzengine {

struct Allocator {
    virtual ~Allocator() {};

    virtual void* allocate(sizet size, sizet alignment) = 0;
    virtual void  deallocate(void* pointer)             = 0;
};

constexpr sizet giga(sizet gigs) {
    return gigs * 1024 * 1024 * 1024; // NOLINT
}

constexpr sizet mega(sizet gigs) {
    return gigs * 1024 * 1024; // NOLINT
}

constexpr sizet kilo(sizet gigs) {
    return gigs * 1024; // NOLINT
}

class Arena : public Allocator {
  public:
    Arena()  = default; // Default constructor, no initialization
    ~Arena() = default; // Default destructor, no cleanup

    // Initialize the allocator with a given size (default 1GB)
    bool  init(size_t size = mega(64)); // NOLINT
    // Shutdown and release all resources
    void  shutdown();

    void* allocate(size_t size, size_t alignment) override;
    void  deallocate(void* pointer) override;

  private:
    static const size_t PAGE_SIZE      = mega(1); // Size of dynamicaly allocated pages
    u8*                 base           = nullptr;
    sizet               allocated_size = 0;
    size_t              committed_size = 0;
    size_t              total_size     = 0;
};

struct ArenaFixed : public Allocator {};

struct HeapAllocator : public Allocator {

    void* allocate(sizet size, sizet alignment) override;

    void  deallocate(void* pointer) override;
};

} // namespace fizzengine
#include <foundation/resource_pool.hpp>
#include <spdlog/spdlog.h>
#include <string.h>

namespace fizzengine {

void ResourcePool::init(const ResourcePoolCreation& creation) {
    allocator               = creation.allocator;
    pool_size               = creation.pool_size;
    resource_size           = creation.resource_size;

    sizet bytes_to_allocate = pool_size * (resource_size + sizeof(u32));

    // Ignores alignment of object, split allocation into two if its a big deal
    pool_memory = static_cast<u8*>(allocator->allocate(bytes_to_allocate, 1));
    memset(pool_memory, 0, bytes_to_allocate);

    free_indices   = reinterpret_cast<u32*>(pool_memory + (pool_size * resource_size));
    used_resources = 0;

    for (u32 i = 0; i < pool_size; ++i) {
        free_indices[i] = i;
    }
}

void ResourcePool::shutdown() {

    if (used_resources != 0) {
        spdlog::warn("Resource pool has unfreed resources.\n");
    }

    allocator->deallocate(pool_memory);
}

u32 ResourcePool::obtain_resource() {
    if (used_resources < pool_size) {
        const u32 free_index = free_indices[used_resources++];
        return free_index;
    }
    return k_invalid_index;
}

void ResourcePool::release_resource(u32 handle) {
    free_indices[--used_resources] = handle;
}

void ResourcePool::free_all_resources() {
    used_resources = 0;

    for (uint32_t i = 0; i < pool_size; ++i) {
        free_indices[i] = i;
    }
}

void* ResourcePool::access_resource(u32 handle) {
    if (handle != k_invalid_index) {
        return &pool_memory[handle * resource_size];
    }
    return nullptr;
}

const void* ResourcePool::access_resource(u32 handle) const {
    if (handle != k_invalid_index) {
        return &pool_memory[handle * resource_size];
    }
    return nullptr;
}

} // namespace fizzengine

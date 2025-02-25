#pragma once
#include <foundation/allocators.hpp>
#include <foundation/platform.hpp>

namespace fizzengine {

static const u32 k_invalid_index = 0xffffffff;

struct ResourcePoolCreation {
    Allocator* allocator;
    u32        pool_size;
    u32        resource_size;
};

struct ResourcePool {
    void        init(const ResourcePoolCreation& creation);
    void        shutdown();

    u32         obtain_resource();
    void        release_resource(u32 handle);
    void        free_all_resources();

    void*       access_resource(u32 handle);
    const void* access_resource(u32 handle) const;

    u8*         pool_memory    = nullptr;
    u32*        free_indices   = nullptr;
    Allocator*  allocator      = nullptr;

    u32         used_resources = 0;
    u32         pool_size      = 0;
    u32         resource_size  = 0;
};

template <typename T>
struct Pool : public ResourcePool {

    void     init(Allocator* allocator, u32 pool_size);

    T*       obtain();
    void     release(T* resource);

    T*       get(u32 index);
    const T* get(u32 index) const;
};

template <typename T>
inline void Pool<T>::init(Allocator* allocator, u32 pool_size_) {
    ResourcePool::init(
        {.allocator = allocator, .pool_size = pool_size, .resource_size = sizeof(T)});
}

template <typename T>
inline T* Pool<T>::obtain() {
    u32 resource_index = ResourcePool::obtain_resource();
    if (resource_index != k_invalid_index) {
        T* resource          = get(resource_index);
        resource->pool_index = resource_index;
        return resource;
    }

    return nullptr;
}

template <typename T>
inline void Pool<T>::release(T* resource) {
    ResourcePool::release_resource(resource->pool_index);
}

template <typename T>
inline T* Pool<T>::get(u32 index) {
    return (T*)ResourcePool::access_resource(index);
}

template <typename T>
inline const T* Pool<T>::get(u32 index) const {
    return (const T*)ResourcePool::access_resource(index);
}

} // namespace fizzengine
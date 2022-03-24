#include "stdafx.h"

#include "voxel/chunk.h"

#include "voxel/chunk/allocator.h"

void hvox::ChunkAllocator::dispose() {
    std::lock_guard<std::mutex> lock(m_handles_mutex);

    for (auto& handle : m_handles) {
        try_deallocate(handle.second);
    }
    ChunkHandles().swap(m_handles);
}

hvox::ChunkHandle hvox::ChunkAllocator::acquire(ChunkGridPosition pos) {
    return ChunkAllocator::acquire(pos.id);
}

hvox::ChunkHandle hvox::ChunkAllocator::acquire(ChunkID id) {
    std::lock_guard<std::mutex> lock(m_handles_mutex);

    auto it = m_handles.find(id);
    if (it != m_handles.end()) {
        return it->second;
    } else {
        return allocate(id);
    }
}

bool hvox::ChunkAllocator::release(ChunkHandle&& handle) {
    std::lock_guard<std::mutex> lock(m_handles_mutex);

    if (handle == nullptr) return false;

    auto it = m_handles.find(handle->position.id);
    if (it != m_handles.end()) {
        return try_deallocate(it->second);
    } else {
        // We should not reach here.
        assert(false);
        return false;
    }
}
#include "stdafx.h"

#include "voxel/block.hpp"

#include "voxel/chunk.h"

hvox::Chunk::Chunk() :
    neighbours({}),
    blocks(nullptr),
    state(ChunkState::NONE),
    pending_task(ChunkTaskKind::NONE)
{ /* Empty. */ }

hvox::Chunk::~Chunk() {
    // debug_printf("Unloading chunk at (%d, %d, %d).\n", position.x, position.y, position.z);

    m_block_pager->free_page(blocks);
    blocks = nullptr;

    instance.dispose();

    neighbours = {};
}

void hvox::Chunk::init(
                 hmem::WeakHandle<Chunk> self,
           hmem::Handle<ChunkBlockPager> block_pager,
    hmem::Handle<ChunkInstanceDataPager> instance_data_pager
) {
    init_events(self);

    blocks = block_pager->get_page();
    m_block_pager = block_pager;

    instance.init(instance_data_pager);

    neighbours = {};

    state.store(ChunkState::PRELOADED, std::memory_order_release);
}

void hvox::Chunk::update(FrameTime) {
    // Empty for now.
}

void hvox::Chunk::init_events(hmem::WeakHandle<Chunk> self) {
    on_block_change         .set_sender(Sender(self));
    on_bulk_block_change    .set_sender(Sender(self));
    on_load                 .set_sender(Sender(self));
    on_mesh_change          .set_sender(Sender(self));
    on_render_state_change  .set_sender(Sender(self));
    on_unload               .set_sender(Sender(self));
}

bool hvox::set_block( hmem::Handle<Chunk> chunk,
                       BlockChunkPosition block_position,
                                    Block block )
{
    auto block_idx = block_index(block_position);

    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
        if (!gen_task_active) {
            bool should_cancel = chunk->on_block_change({
                chunk,
                chunk->blocks[block_idx],
                block,
                block_position
            });
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    chunk->blocks[block_idx] = block;

    return true;
}

bool hvox::set_blocks( hmem::Handle<Chunk> chunk,
                        BlockChunkPosition start_block_position,
                        BlockChunkPosition end_block_position,
                                     Block block )
{
    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
        if (!gen_task_active) {
            bool should_cancel = chunk->on_bulk_block_change({
                chunk,
                &block,
                true,
                start_block_position,
                end_block_position
            });
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    set_per_block_data(
        chunk->blocks,
        start_block_position,
        end_block_position,
        block
    );

    return true;
}

bool hvox::set_blocks( hmem::Handle<Chunk> chunk,
                        BlockChunkPosition start_block_position,
                        BlockChunkPosition end_block_position,
                                    Block* blocks )
{
    {
        std::shared_lock lock(chunk->blocks_mutex);

        bool gen_task_active = chunk->gen_task_active.load(std::memory_order_acquire);
        if (!gen_task_active) {
            bool should_cancel = chunk->on_bulk_block_change({
                chunk,
                blocks,
                false,
                start_block_position,
                end_block_position
            });
            if (should_cancel) return false;
        }
    }

    std::lock_guard lock(chunk->blocks_mutex);

    set_per_block_data(
        chunk->blocks,
        start_block_position,
        end_block_position,
        blocks
    );

    return true;
}

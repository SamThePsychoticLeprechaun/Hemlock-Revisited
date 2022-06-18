#include "graphics/mesh.h"
#include "voxel/block.hpp"
#include "voxel/chunk/grid.h"

static inline bool is_at_left_face(hvox::BlockIndex index) {
    return (index % CHUNK_SIZE) == 0;
}
static inline bool is_at_right_face(hvox::BlockIndex index) {
    return ((index + 1) % CHUNK_SIZE) == 0;
}
static inline bool is_at_bottom_face(hvox::BlockIndex index) {
    return (index % (CHUNK_SIZE * CHUNK_SIZE)) < CHUNK_SIZE;
}
static inline bool is_at_top_face(hvox::BlockIndex index) {
    return (index % (CHUNK_SIZE * CHUNK_SIZE)) >= (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline bool is_at_front_face(hvox::BlockIndex index) {
    return index < (CHUNK_SIZE * CHUNK_SIZE);
}
static inline bool is_at_back_face(hvox::BlockIndex index) {
    return index >= (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}

static inline hvox::BlockIndex index_at_right_face(hvox::BlockIndex index) {
    return index + CHUNK_SIZE - 1;
}
static inline hvox::BlockIndex index_at_left_face(hvox::BlockIndex index) {
    return index - CHUNK_SIZE + 1;
}
static inline hvox::BlockIndex index_at_top_face(hvox::BlockIndex index) {
    return index + (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_bottom_face(hvox::BlockIndex index) {
    return index - (CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_front_face(hvox::BlockIndex index) {
    return index - (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}
static inline hvox::BlockIndex index_at_back_face(hvox::BlockIndex index) {
    return index + (CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1));
}

template <hvox::ChunkMeshComparator MeshComparator>
bool hvox::ChunkNaiveMeshTask<MeshComparator>::run_task(ChunkLoadThreadState* state, ChunkLoadTaskQueue* task_queue) {
    auto chunk = m_chunk.lock();

    if (chunk == nullptr) return false;

    chunk->mesh_task_active.store(true, std::memory_order_release);

    // Only execute if all preloaded neighbouring chunks have at least been generated.
    auto [ _, neighbours_in_required_state ] =
            m_chunk_grid->query_all_neighbour_states(chunk, ChunkState::GENERATED);

    if (!neighbours_in_required_state) {
        // Mark as no longer engaging in this meshing task.
        chunk->mesh_task_active.store(false, std::memory_order_release);
        // Put copy of this mesh task back onto the load task queue.
        ChunkNaiveMeshTask<MeshComparator>* mesh_task = new ChunkNaiveMeshTask<MeshComparator>();
        mesh_task->set_workflow_metadata(m_tasks, m_task_idx, m_dag, m_task_completion_states);
        mesh_task->init(chunk, m_chunk_grid);
        task_queue->enqueue(state->producer_token, { mesh_task, true });
        chunk->pending_task.store(ChunkLoadTaskKind::MESH, std::memory_order_release);
        return false;
    }

    chunk->instance = { nullptr, 0 };

    // TODO(Matthew): Better guess work should be possible and expand only when needed.
    //                  Maybe in addition to managing how all chunk's transformations are
    //                  stored on GPU, ChunkGrid-level should also manage this data?
    //                    This could get hard with scalings as well (as will come from
    //                    something like a greedy "meshing" algorithm).
    // TODO(Matthew):       For greedy meshing, while translations will by definition be
    //                      unique, scalings will not be, and so an index buffer could
    //                      further improve performance and also remove the difficulty
    //                      of the above TODO.
    chunk->instance.data = new ChunkInstanceData[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

    // Determines if block is meshable.
    const MeshComparator meshable{};

    auto add_block = [&](BlockWorldPosition pos) {
        chunk->instance.data[chunk->instance.count++]
                                        = { f32v3(pos), f32v3(1.0f) };
    };

    // TODO(Matthew): Checking block is NULL_BLOCK is wrong check really, we will have transparent blocks
    //                e.g. air, to account for too.
    for (BlockIndex i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ++i) {
        Block& voxel = chunk->blocks[i];
        if (voxel != NULL_BLOCK) {
            BlockWorldPosition block_position = block_world_position(chunk->position, i);

            hmem::Handle<Chunk> neighbour;

            // Check its neighbours, to decide whether to add its quads.
            // LEFT
            if (is_at_left_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_right_face(i);
                neighbour = chunk->neighbours.one.left.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i - 1], &chunk->blocks[i - 1], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }

            // RIGHT
            if (is_at_right_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_left_face(i);
                neighbour = chunk->neighbours.one.right.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i + 1], &chunk->blocks[i + 1], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }

            // BOTTOM
            if (is_at_bottom_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_top_face(i);
                neighbour = chunk->neighbours.one.bottom.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i - CHUNK_SIZE], &chunk->blocks[i - CHUNK_SIZE], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }

            // TOP
            if (is_at_top_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_bottom_face(i);
                neighbour = chunk->neighbours.one.top.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i + CHUNK_SIZE], &chunk->blocks[i + CHUNK_SIZE], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }

            // FRONT
            if (is_at_front_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_back_face(i);
                neighbour = chunk->neighbours.one.front.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i - (CHUNK_SIZE * CHUNK_SIZE)], &chunk->blocks[i - (CHUNK_SIZE * CHUNK_SIZE)], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }

            // BACK
            if (is_at_back_face(i)) {
                // Get corresponding neighbour index in neighbour chunk and check.
                BlockIndex j = index_at_front_face(i);
                neighbour = chunk->neighbours.one.back.lock();
                if (neighbour == nullptr || neighbour->blocks[j] == NULL_BLOCK) {
                    add_block(block_position);
                    continue;
                }
            } else {
                // Get corresponding neighbour index in this chunk and check.
                if (meshable(&chunk->blocks[i + (CHUNK_SIZE * CHUNK_SIZE)], &chunk->blocks[i + (CHUNK_SIZE * CHUNK_SIZE)], block_chunk_position(i), chunk)) {
                    add_block(block_position);
                    continue;
                }
            }
        }
    }

    chunk->state.store(ChunkState::MESHED, std::memory_order_release);

    chunk->mesh_task_active.store(false, std::memory_order_release);

    chunk->on_mesh_change();

    // TODO(Matthew): Set next task if chunk unload is false? Or else set that
    //                between this task and next, but would need adjusting
    //                workflow.
    chunk->pending_task.store(ChunkLoadTaskKind::NONE, std::memory_order_release);

    return !chunk->unload.load(std::memory_order_acquire);
}

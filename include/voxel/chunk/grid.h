#ifndef __hemlock_voxel_chunk_grid_h
#define __hemlock_voxel_chunk_grid_h

#include "timing.h"
#include "voxel/chunk.h"

namespace hemlock {
    namespace voxel {
        // TODO(Matthew): We can do better storage of chunks and blocks at this level.
        // TODO(Matthew): We should add support for LOD, different generation stages, disabling meshing etc.

        using Chunks = std::unordered_map<ChunkID, Chunk*>;

        using QueriedChunkState       = std::pair<bool, bool>;
        using QueriedChunkPendingTask = std::pair<bool, bool>;

        class ChunkGrid {
        public:
            ChunkGrid()  { /* Empty. */ }
            ~ChunkGrid() { /* Empty. */ }

            /**
             * @brief Initialises the chunk grid and the
             * underlying thread pool.
             *
             * @param thread_count The number of threads
             * that the grid can use for loading tasks.
             */
            void init(ui32 thread_count);
            /**
             * @brief Disposes of the chunk grid, ending
             * the tasks on the thread pool and unloading
             * all chunks.
             */
            void dispose();

            /**
             * @brief Update loop for chunks.
             *
             * @param time The time data for the frame.
             */
            void update(TimeData time);
            /**
             * @brief Draw loop for chunks.
             *
             * @param time The time data for the frame.
             */
            void draw(TimeData time);

            /**
             * @brief Suspends chunk tasks. This is a hammer, but
             * for testing it can definitely be useful. Probably
             * don't ever call this in practise.
             */
            void suspend_chunk_tasks() { m_gen_threads.suspend(); }
            /**
             * @brief Resumes chunk tasks. No consequences for
             * calling this when not already suspended.
             */
            void resume_chunk_tasks()  { m_gen_threads.resume();  }

            /**
             * @brief Loads chunks with the assumption none specified
             * have even been preloaded. This is useful as it assures
             * all preloading is done before any loading so that there
             * is no need for corrective load tasks later for adjoining
             * chunks etc.
             *
             * @param chunk_positions The array of chunk coords for which
             * to load chunks.
             * @param chunk_count The number of chunks to load.
             * @return True if all chunks have got to the point of their
             * load tasks being queued in a valid state, false if any single
             * chunk did not.
             */
            bool load_from_scratch_chunks(ChunkGridPosition* chunk_positions, ui32 chunk_count);

            /**
             * @brief Preloads a chunk, this entails saying it exists
             * and determining its neighbours - letting it and them
             * know if each other's existence.
             *
             * @param chunk_position The coords of the chunk to preload.
             * @return True if the chunk was preloaded, false otherwise.
             * False usually will mean that the chunk was at least already
             * in a preloaded state.
             */
            bool preload_chunk_at(ChunkGridPosition chunk_position);
            /**
             * @brief Loads a chunk, this entails queuing a generation
             * task which will, by default, queue a meshing task to
             * follow up.
             *
             * @param chunk_position The coords of the chunk to load.
             * @return True if the chunk's load task was queued, false
             * otherwise. False usually will mean that the chunk was
             * either not yet preloaded, or at least already in a loaded
             * state.
             */
            bool load_chunk_at(ChunkGridPosition chunk_position);
            /**
             * @brief Unloads a chunk, this entails ending all
             * pending tasks for this chunk and releasing memory
             * associated with it.
             *
             * @param chunk_position The coords of the chunk to unload.
             * @return True if the chunk was unloaded, false otherwise.
             * False usually will mean that the chunk was not yet
             * existent, as if it is in any existing state some degree
             * of work will be done to unload it.
             */
            bool unload_chunk_at(ChunkGridPosition chunk_position);

            /**
             * @brief Queries the state of the chunk at the given
             * position. The requirement verified here is that
             * the so-positioned chunk is at the very least in
             * the specified state; "later" states shall also
             * satisfy the requirement here.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_minimum_state The minimum state
             * required of the chunk.
             * @return [true, true] if the chunk is at least in the
             * required state, [true, false] if the chunk exists
             * but does not satisfy the state requirement,
             * [false, false] if the chunk does not exist. Note:
             * [false, true] should never occur and represents
             * invalid query processing.
             */
            QueriedChunkState query_chunk_state(ChunkGridPosition chunk_position, ChunkState required_minimum_state);
            /**
             * @brief Queries the state of the chunk. The
             * requirement verified here is that the chunk is
             * at the very least in the specified state;
             * "later" states shall also satisfy the
             * requirement here.
             *
             * @param chunk The chunk to query
             * @param required_minimum_state The minimum state
             * required of the chunk.
             * @return [true, true] if the chunk is at least in the
             * required state, [true, false] if the chunk exists
             * but does not satisfy the state requirement,
             * [false, false] if the chunk does not exist. Note:
             * [false, true] should never occur and represents
             * invalid query processing.
             */
            QueriedChunkState query_chunk_state(Chunk* chunk, ChunkState required_minimum_state);

            /**
             * @brief Queries the pending task of the chunk
             * at the given position. The requirement verified
             * here is that the so-positioned chunk is at the
             * very least pending going into the specified
             * task; "later" tasks shall also satisfy the
             * requirement here as that implies the task
             * has already been achieved.
             *
             * @param chunk_position The position of the chunk
             * to query.
             * @param required_minimum_pending_task The minimum
             * task required of the chunk.
             * @return [true, true] if the chunk is at least
             * pending the required task, [true, false] if the
             * chunk exists but does not satisfy the pending
             * task requirement, [false, false] if the chunk
             * does not exist. Note: [false, true] should
             * never occur and represents invalid query
             * processing.
             */
            QueriedChunkPendingTask query_chunk_pending_task(ChunkGridPosition chunk_position, ChunkLoadTaskKind required_minimum_pending_task);
            /**
             * @brief Queries the pending task of the chunk
             *  The requirement verified here is that the
             * chunk is at the very least pending going into
             * the specified task; "later" tasks shall also
             * satisfy the requirement here as that implies
             * the task has already been achieved.
             *
             * @param chunk The chunk to query
             * @param required_minimum_pending_task The minimum
             * task required of the chunk.
             * @return [true, true] if the chunk is at least
             * pending the required task, [true, false] if the
             * chunk exists but does not satisfy the pending
             * task requirement, [false, false] if the chunk
             * does not exist. Note: [false, true] should
             * never occur and represents invalid query
             * processing.
             */
            QueriedChunkPendingTask query_chunk_pending_task(Chunk* chunk, ChunkLoadTaskKind required_minimum_pending_task);

            /**
             * @brief Queries the state of the neighbours of
             * the chunk at the given position. The requirement
             * verified here is that the neighbours of the
             * so-positioned chunk are at the very least in
             * the specified state; "later" states shall also
             * satisfy the requirement here.
             *
             * @param chunk_position The position of the chunk
             * whose neighbours are to be queried.
             * @param required_minimum_state The minimum state
             * required of the chunks.
             * @return [true, true] if the chunks are at least in the
             * required state, [true, false] if the chunk whose
             * neighbours we are querying exists but its neighbours
             * do not satisfy the state requirement, [false, false]
             * if the chunk whose neighbours we are querying does
             * not exist. Note: [false, true] should never occur
             * and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_states(ChunkGridPosition chunk_position, ChunkState required_minimum_state);
            /**
             * @brief Queries the state of the neighbours of
             * the chunk. The requirement verified here is
             * that the neighbours of the chunk are at the
             * very least in the specified state; "later"
             * states shall also satisfy the requirement here.
             *
             * @param chunk The chunk whose neighbours are to
             * be queried.
             * @param required_minimum_state The minimum state
             * required of the chunks.
             * @return [true, true] if the chunks are at least in the
             * required state, [true, false] if the chunk whose
             * neighbours we are querying exists but its neighbours
             * do not satisfy the state requirement, [false, false]
             * if the chunk whose neighbours we are querying does
             * not exist. Note: [false, true] should never occur
             * and represents invalid query processing.
             */
            QueriedChunkState query_all_neighbour_states(Chunk* chunk, ChunkState required_minimum_state);

            const Chunks& chunks() { return m_chunks; }
        protected:
            void establish_chunk_neighbours(Chunk* chunk);

            ThreadPool<ChunkLoadTaskContext> m_gen_threads;

            // TODO(Matthew): this probably should go elsewhere later, but
            //                to get instancing working this will do.
            hg::MeshHandles   m_mesh_handles;
            GLuint            m_instance_vbo;
            bool              m_TEMP_not_all_ready;
            ui32              m_voxel_count;

            Chunks m_chunks;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_grid_h
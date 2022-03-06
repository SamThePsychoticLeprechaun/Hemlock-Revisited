#ifndef __hemlock_voxel_chunk_load_task_hpp
#define __hemlock_voxel_chunk_load_task_hpp

namespace hemlock {
    namespace voxel {
        struct Chunk;
        class ChunkGrid;

        enum class ChunkLoadTaskKind : ui8 {
            NONE            = 0,
            GENERATION      = 1,
            GENERATION_0    = 1,
            GENERATION_1       ,
            GENERATION_2       ,
            GENERATION_3       ,
            GENERATION_4       ,
            GENERATION_5       ,
            GENERATION_6       ,
            GENERATION_7       ,
            GENERATION_8       ,
            GENERATION_9       ,
            GENERATION_10      ,
            GENERATION_11      ,
            GENERATION_12      ,
            GENERATION_13      ,
            GENERATION_14      ,
            GENERATION_15      ,
            MESH               ,
            MESH_UPLOAD        ,
        };

        struct ChunkLoadTaskContext {
            volatile bool stop;
            volatile bool suspend;
        };
        using ChunkLoadThreadState = Thread<ChunkLoadTaskContext>::State;
        using ChunkLoadTaskQueue   = TaskQueue<ChunkLoadTaskContext>;
        class ChunkLoadTask : public IThreadTask<ChunkLoadTaskContext> {
        public:
            void init(Chunk* chunk, ChunkGrid* chunk_grid);
        protected:
            Chunk*     m_chunk;
            ChunkGrid* m_chunk_grid;
        };
    }
}
namespace hvox = hemlock::voxel;

#endif // __hemlock_voxel_chunk_load_task_hpp

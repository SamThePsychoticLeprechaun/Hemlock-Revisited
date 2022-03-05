#ifndef __hemlock_thread_thread_workflow_state_hpp
#define __hemlock_thread_thread_workflow_state_hpp

#include "thread_pool.hpp"

namespace hemlock {
    namespace thread {
        template <hthread::InterruptibleState ThreadState>
        class IThreadWorkflowTask;

        template <hthread::InterruptibleState ThreadState>
        struct HeldWorkflowTask {
            IThreadWorkflowTask<ThreadState>* task;
            bool should_delete;
        };

        using ThreadWorkflowTaskID = i32;

        template <hthread::InterruptibleState ThreadState>
        using ThreadWorkflowTaskList = std::span<HeldWorkflowTask<ThreadState>>;

        using ThreadWorkflowTaskCompletion     =           std::atomic<ui32> ;
        using ThreadWorkflowTaskCompletionView = std::span<std::atomic<ui32>>;

        using ThreadWorkflowTaskIntoCount = std::vector<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskIndexList = std::unordered_set<ThreadWorkflowTaskID>;
        using ThreadWorkflowTaskGraph     = std::unordered_multimap<ThreadWorkflowTaskID, ThreadWorkflowTaskID>;

        struct ThreadWorkflowDAG {
            ThreadWorkflowTaskIntoCount into_counts;
            ThreadWorkflowTaskIndexList entry_tasks;
            ThreadWorkflowTaskGraph     graph;
        };
    }
}
namespace hthread = hemlock::thread;

#endif // __hemlock_thread_thread_workflow_state_hpp

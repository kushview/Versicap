
#include "exporters/ExportThread.h"
#include "Project.h"

namespace vcp {

ExportThread::ExportThread()
    : Thread ("vcpexport"),
      started (*this), finished (*this),
      progressNotify (*this)
{
    startThread();
}

ExportThread::~ExportThread()
{
    cancel();
}

Result ExportThread::start (Versicap& versicap, const Project& project)
{
    if (! state.compareAndSetBool (Preparing, Idle))
        return Result::fail ("Export already in progress");
    
    OwnedArray<ExportTask> newTasks;
    project.getExportTasks (newTasks);
    for (auto* task : newTasks)
        task->prepare (versicap);

    {
        ScopedLock sl (lock);
        progress = 0.0;
        progressTitle = String();
        tasks.swapWith (newTasks);
    }

    notify();
    newTasks.clear (true);

    return Result::ok();
}

void ExportThread::cancel()
{
    signalThreadShouldExit();
    notify();
    stopThread (5000);
}

void ExportThread::run()
{
    while (! threadShouldExit())
    {
        state.set (Idle);
        wait (-1);
        
        if (threadShouldExit())
            break;

        state.set (Running);

        Thread::sleep (500);

        for (int i = 0; i < tasks.size(); ++i)
        {
            auto* const task = tasks.getUnchecked (i);
        
            {
                ScopedLock sl (lock);
                progress = static_cast<double> (i + 1) / static_cast<double> (tasks.size());
                progressTitle = task->getProgressName();
                progressNotify.triggerAsyncUpdate();
            }
        
            const auto result = task->perform();

            if (threadShouldExit())
                break;

            if (! result.wasOk())
            {
                DBG("[VCP] " << result.getErrorMessage());
                // trigger error and cancel
                break;
            }
        }

        Thread::sleep (500);

        state.set (Finished);
        finished.triggerAsyncUpdate();

        if (threadShouldExit())
            break;
    }
}

}

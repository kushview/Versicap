
#include "exporters/ExportThread.h"
#include "Project.h"

namespace vcp {

ExportThread::ExportThread()
    : Thread ("vcpexport")
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

    tasks.swapWith (newTasks);
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

void ExportThread::handleAsyncUpdate()
{
    static int counter = 0;
    DBG("count: " << counter++);

    switch (state.get())
    {
        case Idle:
            DBG("[VCP] export is Idle");
            break;
        case Running:
            DBG("[VCP] export is Running");
            break;
        case Finished:
            DBG("[VCP] export is Finished");
            state.set (Idle);
            break;
    }
}

void ExportThread::run()
{
    while (! threadShouldExit())
    {
        state.set (Idle);
        triggerAsyncUpdate();
        wait (-1);
        
        if (threadShouldExit())
            break;

        state.set (Running);

        for (auto* const task : tasks)
        {
            const auto result = task->perform();
            if (! result.wasOk())
            {
                DBG("[VCP]" << result.getErrorMessage());
                // trigger error and cancel
                break;
            }
        }

        state.set (Finished);
        triggerAsyncUpdate();
    }
}

}

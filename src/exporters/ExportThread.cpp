
#include "exporters/ExportThread.h"

namespace vcp
{

ExportThread::ExportThread()
    : Thread ("vcpexport")
{
    startThread();
}

ExportThread::~ExportThread()
{
    cancel();
}

Result ExportThread::start (const Project& project)
{
    if (isExporting())
        return Result::fail ("Export already in progress");
    
    OwnedArray<ExportTask> newTasks;
    project.getExportTasks (newTasks);

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

        for (auto* const task : tasks)
        {
            
        }

        state.set (Running);
        triggerAsyncUpdate();
    }

    state.set (Finished);
    triggerAsyncUpdate();
}

}

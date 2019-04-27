
#pragma once

#include "exporters/Exporter.h"

namespace vcp {

class Project;

class ExportThread : private Thread,
                     private AsyncUpdater
{
public:
    ExportThread();
    ~ExportThread();

    Result start (const Project& project);
    bool isExporting() const { return isThreadRunning(); }
    void cancel();

private:
    enum State
    {
        Idle    = 0,
        Running,
        Finished
    };

    Atomic<int> state { Idle };

    OwnedArray<ExportTask> tasks;
    void run() override;
    void handleAsyncUpdate() override;
};

}

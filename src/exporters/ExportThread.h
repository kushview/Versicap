
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

    std::function<void()> onStarted;
    std::function<void()> onFinished;
    std::function<void()> onProgress;

    Result start (Versicap& versicap, const Project& project);
    bool isExporting() const { return state.get() != Idle; }
    void cancel();

    String getProgressTitle() const
    {
        ScopedLock sl (lock);
        return progressTitle;
    }

    double getProgress() const 
    {
        ScopedLock sl (lock);
        return progress;
    }

private:
    enum State
    {
        Idle    = 0,
        Preparing,
        Running,
        Finished
    };

    Atomic<int> state { Idle };
    CriticalSection lock;
    double progress = 0.0;
    String progressTitle;

    OwnedArray<ExportTask> tasks;
    void run() override;
    void handleAsyncUpdate() override;

    struct Started : public AsyncUpdater
    {
        Started (ExportThread& t) : thread (t) {}
        void handleAsyncUpdate() override { if (thread.onStarted) thread.onStarted(); }
        ExportThread& thread;
    } started;

    struct Finished : public AsyncUpdater
    {
        Finished (ExportThread& t) : thread (t) {}
        void handleAsyncUpdate() override { if (thread.onFinished) thread.onFinished(); }
        ExportThread& thread;
    } finished;

    struct Progress : public AsyncUpdater
    {
        Progress (ExportThread& t) : thread (t) {}
        void handleAsyncUpdate() override { if (thread.onProgress) thread.onProgress(); }
        ExportThread& thread;
    } progressNotify;
};

}

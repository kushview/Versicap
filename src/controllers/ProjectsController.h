#pragma once

#include "controllers/Controller.h"
#include "ProjectWatcher.h"

namespace vcp {

class ProjectDocument;

class ProjectsController : public Controller,
                           private ChangeListener
{
public:
    ProjectsController (Versicap& vc);
    ~ProjectsController();

    String getName() const override { return "Projects"; }
    
    bool hasProjectChanged() const;

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const ApplicationCommandTarget::InvocationInfo&) override;
    
    void projectChanged() override;

    void initialize() override;
    void shutdown() override;

    void createNewProject (const String& alertTitle);

private:
    std::unique_ptr<ProjectDocument> document;
    ProjectWatcher watcher;

    void save();
    void saveAs();
    void open();
    void create();
    void record();
    
    void updateEngineContext();
    
    void changeListenerCallback (ChangeBroadcaster*) override;
};

}

#pragma once

#include "controllers/Controller.h"

namespace vcp {
class ProjectsController : public Controller
{
public:
    ProjectsController (Versicap& vc)
        : Controller (vc) { }
    ~ProjectsController() = default;

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const ApplicationCommandTarget::InvocationInfo&) override;

private:
    void save (bool saveAs = false);
};

}

#pragma once

#include "controllers/Controller.h"

namespace vcp {
class ProjectsController : public Controller
{
public:
    ProjectsController (Versicap& vc)
        : Controller (vc) { }
    ~ProjectsController() = default;
};
}

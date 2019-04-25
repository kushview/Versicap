#pragma once

#include "Project.h"

namespace vcp {

class Versicap;

class ProjectConcertinaPanel : public ConcertinaPanel
{
public:
    ProjectConcertinaPanel();
    ~ProjectConcertinaPanel();
    void createPanels (Versicap&);
    
};

}

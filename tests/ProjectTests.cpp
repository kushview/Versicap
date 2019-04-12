#include "Tests.h"

namespace vcp {

class ProjectTests : public UnitTestBase
{
public:
    ProjectTests() : UnitTestBase ("Projects", "model", "project") {}
    
    void runTest() override
    {
        beginTest ("properties");
        Project project;
        
        project.getFormatType() == FormatType::WAVE;
        project.getFormatTypeSlug() == FormatType::getSlug (project.getFormatType());
    }
};

static ProjectTests sProjectTests;

}
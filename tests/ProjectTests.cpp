#include "Tests.h"

namespace vcp {

class ProjectTests : public UnitTestBase
{
public:
    ProjectTests() : UnitTestBase ("Project", "model", "project") {}
    
    void runTest() override
    {
        beginTest ("properties");
        Project project;
        expect (project.getNumSampleSets() == 0);
        expect (project.getFormatType() == FormatType::WAVE);
        expect (project.getFormatTypeSlug() == FormatType::getSlug (project.getFormatType()));
    }
};

static ProjectTests sProjectTests;

}
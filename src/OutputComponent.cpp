
#include "MainComponent.h"
#include "OutputComponent.h"

namespace vcp {

void OutputComponent::startRendering()
{
    if (auto* const main = findParentComponentOfClass<MainComponent>())
        main->startRendering();
}

}


#include "EngineComponent.h"
#include "MainComponent.h"

namespace vcp {

void EngineComponent::startRender()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
        main->startRendering();
}

}

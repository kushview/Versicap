#pragma once

#include "Exporter.h"

#if JUCE_MAC
 #include "exporters/EXS24Exporter.h"
#endif

namespace vcp {

void Exporter::createExporters (OwnedArray<Exporter>& exporters)
{
    exporters.add (new EXS24Exporter());
}

}

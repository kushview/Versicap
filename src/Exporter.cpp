
#include "exporters/GenericExporter.h"

#if JUCE_MAC
 #include "exporters/EXS24Exporter.h"
#endif

#include "Exporter.h"

namespace vcp {

void Exporter::createExporters (OwnedArray<Exporter>& exporters)
{
    exporters.add (new GenericExporter());
   #if JUCE_MAC
    exporters.add (new EXS24Exporter());
   #endif
}

}

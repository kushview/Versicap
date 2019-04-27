
#include "exporters/Exporter.h"

namespace vcp {

void ExporterType::createAllTypes (ExporterTypeArray& types)
{
    types.add (createWavExporterType());
}

}

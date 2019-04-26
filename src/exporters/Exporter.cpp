
#include "exporters/Exporter.h"

namespace vcp {

void ExporterType::createAllTypes (OwnedArray<ExporterType>& types)
{
    types.add (createWavExporterType());
}

}

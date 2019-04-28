
#include "exporters/Exporter.h"
#include "Versicap.h"

namespace vcp {

void ExporterType::createAllTypes (Versicap& versicap, ExporterTypeArray& types)
{
    types.add (createWavExporterType (versicap.getAudioFormats()));
    types.add (createAiffExporterType (versicap.getAudioFormats()));
    types.add (createFlacExporterType (versicap.getAudioFormats()));
    types.add (createOggExporterType (versicap.getAudioFormats()));
}

}


#include "exporters/Exporter.h"
#include "Versicap.h"

namespace vcp {

ValueTree ExporterType::createModelData() const
{
    ValueTree data (Tags::exporter);
    data.setProperty (Tags::uuid,    Uuid().toString(), nullptr)
        .setProperty (Tags::object,  const_cast<ExporterType*> (this), nullptr)
        .setProperty (Tags::name,    getName(), nullptr)
        .setProperty (Tags::type,    getSlug(), nullptr)
        .setProperty (Tags::path,    "", nullptr)
        .setProperty (Tags::sets,  "all", nullptr);
    setMissingProperties (data);
    return data;
}

void ExporterType::createAllTypes (Versicap& versicap, ExporterTypeArray& types)
{
    types.add (createWavExporterType (versicap.getAudioFormats()));
    types.add (createAiffExporterType (versicap.getAudioFormats()));
    types.add (createFlacExporterType (versicap.getAudioFormats()));
    types.add (createOggExporterType (versicap.getAudioFormats()));
}

}


#include "exporters/Exporter.h"

namespace vcp {

class WavExporterType : public ExporterType
{
public:
    WavExporterType()
    {

    }

    ~WavExporterType()
    {

    }

    String getSlug() const override { return "wave"; }
    String getName() const override { return "WAVE Exporter"; }
};

ExporterType* ExporterType::createWavExporterType() { return new WavExporterType(); }

}

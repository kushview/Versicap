#pragma once

#include "exporters/Exporter.h"

namespace vcp {

class EXS24Exporter : public Exporter
{
public:
    EXS24Exporter() = default;
    ~EXS24Exporter() = default;

    String getName() const override { return "EXS24"; }
    String getDescription() const override { return "Format for Logic Pro and AUSampler"; }
};

}

#pragma once

#include "../Exporter.h"

namespace vcp {

class EXS24Exporter : public Exporter
{
public:
    EXS24Exporter() = default;
    ~EXS24Exporter() = default;

    String getName() const override { return "EXS24"; }
};

}

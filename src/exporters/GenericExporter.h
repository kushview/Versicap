#pragma once

#include "../Exporter.h"

namespace vcp {

class GenericExporter : public Exporter
{
public:
    GenericExporter() = default;
    ~GenericExporter() = default;

    String getName() const override { return "Generic"; }
    String getDescription() const { return "Generic sample only format"; }
};

}

#pragma once

#include "gui/ContentView.h"

namespace vcp {

class SampleEditContentView : public ContentView
{
public:
    SampleEditContentView (Versicap& vc);
    void resized() override;

private:
    friend class Content; class Content;
    std::unique_ptr<Content> content;
};

}
#pragma once

#include "gui/ContentView.h"
#include "Versicap.h"

namespace vcp {

class SampleEditContentView : public ContentView,
                              public Versicap::Listener
{
public:
    SampleEditContentView (Versicap& vc);
    ~SampleEditContentView();
    void resized() override;
    void projectChanged() override;

private:
    friend class Content; class Content;
    std::unique_ptr<Content> content;
};

}

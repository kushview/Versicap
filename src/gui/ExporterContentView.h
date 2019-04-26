#include "gui/ContentView.h"

namespace vcp {

class ExporterContentView : public ContentView
{
public:
    ExporterContentView (Versicap&);
    ~ExporterContentView();

    void resized() override;

private:
    class Content;
    std::unique_ptr<Content> content;
};

}

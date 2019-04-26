
#include "gui/ContentView.h"

namespace vcp {

class ExportersListContentView : public ContentView
{
public:
    ExportersListContentView (Versicap&);
    ~ExportersListContentView();

    void resized() override;

private:
    class Content;
    std::unique_ptr<Content> content;
};

}

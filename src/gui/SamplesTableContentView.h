
#include "gui/ContentView.h"

namespace vcp {

class SamplesTableContentView : public ContentView
{
public:
    SamplesTableContentView (Versicap&);
    ~SamplesTableContentView();

    void resized() override;

private:
    class Content; std::unique_ptr<Content> content;
};

}

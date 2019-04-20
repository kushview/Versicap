
#include "gui/ContentView.h"
#include "Versicap.h"

namespace vcp {

class SamplesTableContentView : public ContentView,
                                public Versicap::Listener
{
public:
    SamplesTableContentView (Versicap&);
    ~SamplesTableContentView();

    void projectChanged() override;
    void resized() override;

private:
    class Content; std::unique_ptr<Content> content;
};

}

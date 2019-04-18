
#include "gui/ContentView.h"
#include "Versicap.h"

namespace vcp {

class SamplesTableContentView : public PanelContentView,
                                public Versicap::Listener
{
public:
    SamplesTableContentView (Versicap&);
    ~SamplesTableContentView();

    void projectChanged() override;

protected:
    void resizeContent (const Rectangle<int>& area) override;

private:
    class Content; std::unique_ptr<Content> content;
};

}


#include "gui/ContentView.h"
#include "Versicap.h"

namespace vcp {

class LayersTableContentView : public PanelContentView,
                               public Versicap::Listener
{
public:
    LayersTableContentView (Versicap&);
    ~LayersTableContentView();

    void projectChanged() override;

protected:
    void resizeContent (const Rectangle<int>&) override;

private:
    class Content; std::unique_ptr<Content> content;
};

}

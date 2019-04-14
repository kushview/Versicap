
#include "gui/ContentView.h"
#include "Versicap.h"

namespace vcp {

class LayersTableContentView : public ContentView,
                               public Versicap::Listener
{
public:
    LayersTableContentView (Versicap&);
    ~LayersTableContentView();

    void resized() override;

    void projectChanged() override;

private:
    class Content; std::unique_ptr<Content> content;
};

}


#include "gui/ContentView.h"

namespace vcp {

class LayersTableContentView : public ContentView
{
public:
    LayersTableContentView();
    ~LayersTableContentView();

    void resized() override;

private:
    class Content; std::unique_ptr<Content> content;
};

}

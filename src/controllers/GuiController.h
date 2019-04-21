
#include "controllers/Controller.h"

namespace vcp {

class GuiController : public Controller
{
public:
    GuiController (Versicap& vc)
        : Controller (vc) { }
    ~GuiController() = default;
};

}

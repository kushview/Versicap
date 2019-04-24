

#include "controllers/Controller.h"

namespace vcp {

class LookAndFeel;
class MainWindow;

class GuiController : public Controller
{
public:
    GuiController (Versicap& vc);
    ~GuiController();

    String getName() const override { return "GUI"; }
    void initialize() override;
    void shutdown() override;
    void launched() override;

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const ApplicationCommandTarget::InvocationInfo&) override;

private:
    std::unique_ptr<LookAndFeel> look;
    std::unique_ptr<MainWindow> window;
};

}

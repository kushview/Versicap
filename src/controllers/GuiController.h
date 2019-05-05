

#include "controllers/Controller.h"

namespace vcp {

class ContentComponent;
class LookAndFeel;
class MainWindow;

class GuiController : public Controller
{
public:
    GuiController (Versicap& vc);
    ~GuiController();

    String getName() const override { return "GUI"; }

    void displayObject (const ValueTree& object);
    ValueTree getDisplayedObject() const { return displayedObject; }
    
    void initialize() override;
    void shutdown() override;
    void launched() override;

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const ApplicationCommandTarget::InvocationInfo&) override;

private:
    std::unique_ptr<LookAndFeel> look;
    std::unique_ptr<MainWindow> window;
    std::unique_ptr<Component> unlock;
    ValueTree displayedObject;

    ContentComponent* getContent();
    void checkUnlockStatus();
};

}

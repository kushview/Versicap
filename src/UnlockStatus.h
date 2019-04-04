
#pragma once
#include "JuceHeader.h"

namespace vcp {

class Settings;

class UnlockStatus : public kv::EDDOnlineUnlockStatus,
                     public ChangeBroadcaster
{
public:
    UnlockStatus (Settings&);
    ~UnlockStatus() = default;
    String getProductID() override;
    bool doesProductIDMatch (const String& returnedIDFromServer) override;
    RSAKey getPublicKey() override;
    void saveState (const String&) override;
    String getState() override;
    String getWebsiteName() override;
    URL getServerAuthenticationURL() override;
    StringArray getLocalMachineIDs() override;

#if 0
    virtual void userCancelled();
    virtual String getMessageForConnectionFailure (bool isInternetConnectionWorking);
    virtual String getMessageForUnexpectedReply() override;
#endif
private:
    Settings& settings;
    ValueTree props;
};

}

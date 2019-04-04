
#include "Settings.h"
#include "UnlockStatus.h"

#define VCP_LICENSE_SETTINGS_KEY "VCPL"
#define VCP_BASE_URL "http://local.kushview.net"
#define VCP_AUTH_URL VCP_BASE_URL "/edd-cp"
#define VCP_PUBKEY "3,753d95fa8511b392b09e5800043d41d1a7b2d330705f5714dcf2b31c8e22a7e9"
#define VCP_PRIVKEY "4e290ea703612261cb143aaaad7e2be03282bec968eea82b7d064226e66321ab,753d95fa8511b392b09e5800043d41d1a7b2d330705f5714dcf2b31c8e22a7e9"
#define VCP_PRODUCT_ID "784"

namespace vcp {

UnlockStatus::UnlockStatus (Settings& s)
    : settings (s) { }

String UnlockStatus::getProductID() { return VCP_PRODUCT_ID; }
bool UnlockStatus::doesProductIDMatch (const String& returnedIDFromServer) { return getProductID() == returnedIDFromServer; }

RSAKey UnlockStatus::getPublicKey()
{
   #if VCP_PUBKEY_ENCODED
    return RSAKey (Element::getPublicKey());
   #elif defined (VCP_PUBKEY)
    return RSAKey (VCP_PUBKEY);
   #else
    jassertfalse; // you need a public key for unlocking features
    return RSAKey();
   #endif
}

void UnlockStatus::saveState (const String& state)
{
    if (auto* const userProps = settings.getUserSettings())
        userProps->setValue (VCP_LICENSE_SETTINGS_KEY, state);
    if (state.isEmpty()) // clear out our custom flags if no state
        this->props = ValueTree();
}

String UnlockStatus::getState()
{
    if (auto* const upf = settings.getUserSettings())
    {
        const String value =  upf->getValue (VCP_LICENSE_SETTINGS_KEY);
        eddRestoreState (value);
        return value;
    }

    return String();
}

String UnlockStatus::getWebsiteName() { return "Kushview"; }

URL UnlockStatus::getServerAuthenticationURL()
{
    const URL authurl (VCP_AUTH_URL);
    return authurl;
}

StringArray UnlockStatus::getLocalMachineIDs()
{
    auto ids (OnlineUnlockStatus::getLocalMachineIDs());
    return ids;
}

}


#include "Settings.h"
#include "UnlockStatus.h"

#define VCP_LOCAL_AUTH      1

#if VCP_LOCAL_AUTH
 #define VCP_LICENSE_SETTINGS_KEY "VCPL"
 #define VCP_BASE_URL "http://local.kushview.net"
 #define VCP_AUTH_URL VCP_BASE_URL "/edd-cp"
 #define VCP_PUBKEY "5,74a6b445c7b9e603172b6c28302f9858bb95a5524055179f46e3d4ce1051641f"
 #define VCP_PRIVKEY "5d5229d16c94b80278ef89b9c02613791162dc4ff5d5fee8b28ab6bf859c18ad,74a6b445c7b9e603172b6c28302f9858bb95a5524055179f46e3d4ce1051641f"
 #define VCP_PRODUCT_ID "784"
 #define KV_GOLD_ID "1008"

#else
 #define VCP_LICENSE_SETTINGS_KEY "VCPL"
 #define VCP_BASE_URL "https://kushview.net"
 #define VCP_AUTH_URL VCP_BASE_URL "/products/authorize"
 #define VCP_PRODUCT_ID "11733"
 #define KV_GOLD_ID "12692"
 #define VCP_PUBKEY_ENCODED 1

#endif

namespace vcp {

#if VCP_PUBKEY_ENCODED
 #include "PublicKey.h"
#endif

UnlockStatus::UnlockStatus (Settings& s)
    : settings (s) { }

String UnlockStatus::getProductID() { return VCP_PRODUCT_ID; }
bool UnlockStatus::doesProductIDMatch (const String& returnedIDFromServer)
{
    StringArray pids { KV_GOLD_ID, VCP_PRODUCT_ID };
    return pids.contains (returnedIDFromServer);
}

RSAKey UnlockStatus::getPublicKey()
{
   #if VCP_PUBKEY_ENCODED
    return RSAKey (vcp::createStringKey());
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


#include "Settings.h"
#include "UnlockStatus.h"

#define VCP_LOCAL_AUTH      1

#if VCP_LOCAL_AUTH
 #define VCP_LICENSE_SETTINGS_KEY "VCPL"
 #define VCP_BASE_URL "http://local.kushview.net"
 #define VCP_AUTH_URL VCP_BASE_URL "/edd-cp"
 #define VCP_PUBKEY "3,753d95fa8511b392b09e5800043d41d1a7b2d330705f5714dcf2b31c8e22a7e9"
 #define VCP_PRIVKEY "4e290ea703612261cb143aaaad7e2be03282bec968eea82b7d064226e66321ab,753d95fa8511b392b09e5800043d41d1a7b2d330705f5714dcf2b31c8e22a7e9"
 #define VCP_PRODUCT_ID "784"
 #define KV_GOLD_ID "1008"

#else
 #define VCP_LICENSE_SETTINGS_KEY "VCPL"
 #define VCP_BASE_URL "https://kushview.net"
 #define VCP_AUTH_URL VCP_BASE_URL "/products/authorize"
 #define VCP_PUBKEY "3,a40a225a3604e2fe44fab13a60a841fa2d13029e9f9ec26eebefa95131211a5a5454a8a6d5cdf8071cfa28772aefde8e899b2b437ec410327ec4733a82f2030cf67c904ca73896f0fb43327b192e6dc343bac03a6221738c6e8e36ff770cd9d3ff69b8d88b49c34bbc75e847e89510297688a47d0b73eeb8fca52ae7771d0a22c8db40ccb9113a98bb64f2a32733d1e90d49b61bebdc054ac35515a08b11b286d81fa88b628e4c4453aaa0aaca59c39dccd928db1d670841b2c4cfc290450227cdae6a51280b563dff9217da07fac8bca467ba49a68e5a4bf0ebe99badda1e3cbec5991722f038128e2f58d7942b728ea625deb661d0c98db657d99bae12d1b9"
 #define VCP_PRODUCT_ID "11733"
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
    StringArray pids { KV_GOLD_ID };
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

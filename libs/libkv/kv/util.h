#pragma once

#include "./juce.h"

namespace kv {

inline static juce_wchar defaultPasswordChar() noexcept
{
   #if JUCE_LINUX
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

inline static bool canConnectToWebsite (const URL& url, const int timeout = 2000)
{
    std::unique_ptr<InputStream> in (url.createInputStream (false, nullptr, nullptr, String(), timeout, nullptr));
    return in != nullptr;
}

inline static bool areMajorWebsitesAvailable()
{
    const char* urlsToTry[] = {
        "http://google.com",  "http://bing.com",  "http://amazon.com",
        "https://google.com", "https://bing.com", "https://amazon.com", nullptr};

    for (const char** url = urlsToTry; *url != nullptr; ++url)
        if (canConnectToWebsite (URL (*url)))
            return true;

    return false;
}

}

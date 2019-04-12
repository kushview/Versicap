#pragma once

#include "RenderContext.h"
#include "Project.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {

class UnitTestBase : public UnitTest
{
public:
    UnitTestBase (const String& name, const String& category = String(), 
                    const String& _slug = String())
        : UnitTest (name, category), slug (_slug) { }
    
    virtual ~UnitTestBase()
    {
        if (versicap)
        {
            jassertfalse;
            shutdownVersicap();
        }
    }

    const String getId() const { return getCategory().toLowerCase() + "." + getSlug().toLowerCase(); }
    const String& getSlug() const { return slug; }

protected:
    void initializeVersicap()
    {
        if (versicap)
            return;

        versicap.reset (new Versicap ());
        auto& settings = getVersicap().getSettings();
        PropertiesFile::Options opts = settings.getStorageParameters();
        opts.applicationName = "VersicapTests";
        settings.setStorageParameters (opts);
        settings.saveIfNeeded();
    }

    void shutdownVersicap()
    {
        if (! versicap) return;
        versicap.reset();
    }

    void runDispatchLoop (const int millisecondsToRunFor = 40)
    {
        MessageManager::getInstance()->runDispatchLoopUntil (millisecondsToRunFor);
    }
    
    Versicap& getVersicap() { initializeVersicap(); return *versicap; }

private:
    const String slug;
    std::unique_ptr<Versicap> versicap;
};

}
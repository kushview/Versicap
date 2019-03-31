
#include "Versicap.h"
#include "MainComponent.h"
#include "MainTabs.h"

namespace vcp {

class MainComponent::Content : public Component
{
public:
    Content (MainComponent& o)
        : owner (o)
    {
        setOpaque (true);
        addAndMakeVisible (tabs);
        setSize (500, 340);
    }

    ~Content()
    {

    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds());
    }

    MainTabs& getTabs() { return tabs; }

private:
    MainComponent& owner;
    MainTabs tabs;
};

MainComponent::MainComponent (Versicap& vc)
    : versicap (vc)
{
    setOpaque (true);
    content.reset (new Content (*this));
    addAndMakeVisible (content.get());
    setSize (600, 400);

    auto& tabs = content->getTabs();
    tabs.refresh();

    const auto file = File::getSpecialLocation (File::userDesktopDirectory).getChildFile ("default.versicap");
    if (auto* xml = XmlDocument::parse (file))
    {
        auto tree = ValueTree::fromXml (*xml);
        content->getTabs().setCurrentTabIndex ((int) tree.getProperty ("currentTab", 0));

        RenderContext ctx;
        ctx.baseName            = tree.getProperty ("baseName", ctx.baseName);
        ctx.crossfadeLength     = tree.getProperty ("crossfadeLength", ctx.crossfadeLength);;
        ctx.instrumentName      = tree.getProperty ("instrumentName", ctx.instrumentName);;
        ctx.keyEnd              = tree.getProperty ("keyEnd", ctx.keyEnd);;
        ctx.keyStart            = tree.getProperty ("keyStart", ctx.keyStart);;
        ctx.keyStride           = tree.getProperty ("keyStride", ctx.keyStride);;
        ctx.loopEnd             = tree.getProperty ("loopEnd", ctx.loopEnd);;
        ctx.loopMode            = tree.getProperty ("loopMode", ctx.loopMode);;
        ctx.loopStart           = tree.getProperty ("loopStart", ctx.loopStart);;
        ctx.noteLength          = tree.getProperty ("noteLength", ctx.noteLength);;
        ctx.outputPath          = tree.getProperty ("outputPath", ctx.outputPath);;
        ctx.tailLength          = tree.getProperty ("tailLength", ctx.tailLength);;
        auto layers = tree.getChildWithName ("layers");
        
        for (int i = 0; i < 4; ++i)
        {
            auto l = layers.getChild (i);
            ctx.layerEnabled[i]     = (bool) l.getProperty("enabled", ctx.layerEnabled [i]);
            ctx.layerVelocities[i]  = l.getProperty("velocity", ctx.layerVelocities [i]);
        }

        tabs.updateSettings (ctx);
        deleteAndZero (xml);
    }
}

MainComponent::~MainComponent()
{
    auto ctx = content->getTabs().getRenderContext();
    ValueTree tree = ctx.createValueTree();
    tree.setProperty ("currentTab", content->getTabs().getCurrentTabIndex(), nullptr);

    const auto file = File::getSpecialLocation (File::userDesktopDirectory).getChildFile ("default.versicap");
    if (auto* xml = tree.createXml())
    {
        xml->writeToFile (file, String());
        deleteAndZero (xml);
    }

    content.reset();
}

void MainComponent::startRendering()
{
    auto& tabs = content->getTabs();
    auto ctx = tabs.getRenderContext();

    for (int i = 0; i < 4; ++i)
    {
        if (! ctx.layerEnabled [i])
            continue;
        DBG("Layer Enabled (" << int(i + 1) << "): " << (int) ctx.layerEnabled [i]);
        auto* seq = ctx.createMidiMessageSequence (i, 44100.0);
        if (seq)
        {
            for (int i = 0; i < seq->getNumEvents(); ++i)
            {
                auto msg = seq->getEventPointer(i)->message;
                if (msg.isNoteOn())
                {
                    DBG("on:\t" << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4));
                }
                else if (msg.isNoteOff())
                {
                    DBG("off:\t" << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4));
                }
            }

            deleteAndZero (seq);
        }
    }

}

void MainComponent::stopRendering()
{

}

void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (DocumentWindow::backgroundColourId));
}

void MainComponent::resized()
{
    content->setBounds (getLocalBounds());
}

}

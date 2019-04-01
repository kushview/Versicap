
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

    RenderContext ctx;
    File contextFile;
    if (auto* props = versicap.getSettings().getUserSettings())
    {
        tabs.setCurrentTabIndex (props->getIntValue ("currentTab", 0));
        contextFile = props->getFile().getParentDirectory().getChildFile("context.versicap");
        if (contextFile.existsAsFile())
            ctx.restoreFromFile (contextFile);
    }

    tabs.updateSettings (ctx);
}

MainComponent::~MainComponent()
{
    const auto ctx = content->getTabs().getRenderContext();
    File contextFile;

    if (auto* props = versicap.getSettings().getUserSettings())
    {
        props->setValue ("currentTab", content->getTabs().getCurrentTabIndex());
        contextFile = props->getFile().getParentDirectory().getChildFile("context.versicap");
    }

    if (! contextFile.getParentDirectory().exists())
        contextFile.getParentDirectory().createDirectory();

    ctx.writeToFile (contextFile);
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

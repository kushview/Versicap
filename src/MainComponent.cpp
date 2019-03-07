
#include "MainComponent.h"
#include "MainTabs.h"

class MainComponent::Content : public Component
{
public:
    Content (MainComponent& o)
        : owner (o)
    {
        setOpaque (true);
        addAndMakeVisible (tabs);
        setSize (600, 400);
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

private:
    MainComponent& owner;
    MainTabs tabs;
};

MainComponent::MainComponent()
{
    setOpaque (true);
    content.reset (new Content (*this));
    addAndMakeVisible (content.get());
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
    content.reset();
}

void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (DocumentWindow::backgroundColourId));
}

void MainComponent::resized()
{
    content->setBounds (getLocalBounds());
}

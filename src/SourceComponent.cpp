
#include "MainTabs.h"
#include "SourceComponent.h"

void SourceComponent::startRender()
{
    auto* tabs = findParentComponentOfClass<MainTabs>();
    auto ctx = tabs->getRenderContext();

    for (int i = 0; i < 4; ++i)
    {
        if (! ctx.layerEnabled[i])
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
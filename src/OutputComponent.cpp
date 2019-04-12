
#include "MainComponent.h"
#include "OutputComponent.h"

namespace vcp {

void OutputComponent::updateFormatParams()
{
    const auto ext = FormatType::getFileExtension (formatCombo.getSelectedId() - 1);
    if (auto* const format = versicap.getAudioFormats().findFormatForFileExtension (ext))
    {
        auto d = bitDepthCombo.getSelectedId();
        if (d <= 0) d = versicap.getRenderContext().bitDepth;
        auto s = sampleRateCombo.getSelectedId();
        if (s <= 0) s = roundToInt (versicap.getRenderContext().sampleRate);

        sampleRateCombo.clear (dontSendNotification);
        bitDepthCombo.clear (dontSendNotification);

        for (const auto& sr : format->getPossibleSampleRates())
            sampleRateCombo.addItem (String (sr), sr);
        for (const auto& bd : format->getPossibleBitDepths())
            bitDepthCombo.addItem (String (bd) + " bit", bd);

        sampleRateCombo.setSelectedId (s, dontSendNotification);
        if (sampleRateCombo.getSelectedId() <= 0)
            sampleRateCombo.setSelectedId (44100, dontSendNotification);
        bitDepthCombo.setSelectedId (d, dontSendNotification);
        if (bitDepthCombo.getSelectedId() <= 0)
            bitDepthCombo.setSelectedId (16, dontSendNotification);
    }
    else
    {
        for (const auto& sr : Array<int> ({ 44100, 48000, 96000, 192000 }))
            sampleRateCombo.addItem (String (sr), sr);
        for (const auto& bd : Array<int> ({ 16, 24 }))
            bitDepthCombo.addItem (String (bd) + " bit", bd);
        sampleRateCombo.setSelectedItemIndex (0, dontSendNotification);
        bitDepthCombo.setSelectedItemIndex (0, dontSendNotification);
    }
}

}

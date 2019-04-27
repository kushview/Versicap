
#pragma once

#include "JuceHeader.h"

namespace vcp {
namespace Tags {

    static const Identifier active          = "active";
    static const Identifier audioInput      = "audioInput";
    static const Identifier audioInputChannels  = "audioInputChannels";
    
    static const Identifier audioOutput     = "audioOutput";
    static const Identifier audioOutputChannels = "audioOutputChannels";
    static const Identifier baseName        = "baseName";
    static const Identifier bitDepth        = "bitDepth";
    static const Identifier bufferSize      = "bufferSize";
    static const Identifier channels        = "channels";
    static const Identifier dataPath        = "dataPath";
    
    static const Identifier enabled         = "enabled";
    static const Identifier exporters       = "exporters";
    static const Identifier exporter        = "exporter";
    
    static const Identifier file            = "file";
    static const Identifier fileOrId        = "fileOrId";
    static const Identifier format          = "format";
    
    static const Identifier uuid            = "uuid";
    static const Identifier identifier      = "identifier";
    static const Identifier latencyComp     = "latencyComp";
    
    static const Identifier layer           = "layer";
    static const Identifier layers          = "layers";
    static const Identifier length          = "length";
    static const Identifier loop            = "loop";

    static const Identifier midi            = "midi";
    static const Identifier midiChannel     = "midiChannel";
    static const Identifier midiInput       = "midiInput";
    static const Identifier midiOutput      = "midiOutput";
    static const Identifier midiProgram     = "midiProgram";
    
    static const Identifier name            = "name";
    static const Identifier note            = "note";
    static const Identifier notes           = "notes";
    static const Identifier noteStart       = "noteStart";
    static const Identifier noteLength      = "noteLength";
    static const Identifier noteEnd         = "noteEnd";
    static const Identifier noteStep        = "noteStep";

    static const Identifier object          = "object";

    static const Identifier path            = "path";
    static const Identifier plugin          = "plugin";
    static const Identifier project         = "project";

    static const Identifier sample          = "sample";
    static const Identifier samples         = "samples";
    static const Identifier sampleRate      = "sampleRate";

    static const Identifier source          = "source";
    static const Identifier state           = "state";
    static const Identifier tailLength      = "tailLength";

    static const Identifier timeIn          = "timeIn";
    static const Identifier timeOut         = "timeOut";
    static const Identifier type            = "type";

    static const Identifier value           = "value";
    static const Identifier velocity        = "velocity";
    static const Identifier version         = "version";

}
}

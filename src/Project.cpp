
#include "engine/RenderContext.h"
#include "exporters/Exporter.h"
#include "PluginManager.h"
#include "Project.h"
#include "Tags.h"
#include "Types.h"

#define VCP_PROJECT_FILE_VERSION    0

namespace vcp {

//=========================================================================
SampleSet::SampleSet()
    : kv::ObjectModel (Tags::set)
{
    setMissingProperties();
}

SampleSet::SampleSet (const ValueTree& data)
    : kv::ObjectModel (data)
{
    if (objectData.hasType (Tags::set))
        setMissingProperties();
}

String SampleSet::getUuidString() const {   return getProperty (Tags::uuid).toString(); }

Uuid SampleSet::getUuid() const
{
    const Uuid uuid (getUuidString());
    return uuid;
}

bool SampleSet::isValid() const 
{ 
    return objectData.hasType (Tags::set) &&
           objectData.hasProperty (Tags::uuid) &&
           ! Uuid (getProperty (Tags::uuid).toString()).isNull();
}

uint8 SampleSet::getVelocity() const    { return static_cast<uint8> ((int) getProperty (Tags::velocity)); }
int SampleSet::getNoteLength() const    { return (int) getProperty (Tags::noteLength); }
int SampleSet::getTailLength() const    { return (int) getProperty (Tags::tailLength); }
int SampleSet::getMidiChannel() const   { return (int) getProperty (Tags::midiChannel); }
int SampleSet::getMidiProgram() const   { return (int) getProperty (Tags::midiProgram); }

void SampleSet::getSamples (OwnedArray<Sample>& results) const
{
    const Project project (objectData.getParent().getParent());
    project.getSamples (project.indexOf (*this), results);
}

void SampleSet::setMissingProperties()
{
    stabilizePropertyString (Tags::uuid, Uuid().toString());
    stabilizePropertyPOD (Tags::velocity, 127);
    stabilizePropertyPOD (Tags::noteLength, 3000);
    stabilizePropertyPOD (Tags::tailLength, 1000);
    stabilizePropertyPOD (Tags::midiChannel, 1);
    stabilizePropertyPOD (Tags::midiProgram, -1);
}

//=========================================================================
Project::Project()
    : ObjectModel (ValueTree())
{
    setMissingProperties();
}

Project::~Project() { }

//=========================================================================
void Project::setActiveSampleSet (const SampleSet& layer)
{
    auto layers = objectData.getChildWithName (Tags::sets);
    if (layers.getProperty (Tags::active).toString() == layer.getUuidString())
        layers.removeProperty (Tags::active, nullptr);
    layers.setProperty (Tags::active, layer.getUuidString(), nullptr);
}

SampleSet Project::getActiveSampleSet() const
{
    const auto layers = objectData.getChildWithName (Tags::sets);
    const SampleSet layer (find (Tags::sets, Tags::uuid, layers.getProperty (Tags::active)));
    return layer;
}

void Project::setActiveSample (const Sample& sample)
{
    auto samples = objectData.getChildWithName (Tags::samples);
    if (samples.getProperty (Tags::active).toString() == sample.getUuidString())
        samples.removeProperty (Tags::active, nullptr);
    samples.setProperty (Tags::active, sample.getUuidString(), nullptr);
}

Sample Project::getSample (int index) const
{
    auto samples = objectData.getChildWithName (Tags::samples);
    Sample sample (samples.getChild (index));
    return sample;
}

Sample Project::getActiveSample() const
{
    const auto samples = objectData.getChildWithName (Tags::samples);
    const Sample sample (find (Tags::samples, Tags::uuid, samples.getProperty (Tags::active)));
    return sample;
}

//=========================================================================
int Project::getFormatType() const
{
    const int type = FormatType::fromSlug (getFormatTypeSlug());
    jassert (type >= FormatType::Begin && type < FormatType::End);
    return type >= FormatType::Begin && type < FormatType::End 
        ? type : FormatType::WAVE;
}

String Project::getFormatTypeSlug() const
{
    return getProperty (Tags::format, FormatType::getSlug (FormatType::WAVE)).toString();
}

int Project::getSourceType() const
{
    const int type = SourceType::fromSlug (getProperty (Tags::source));
    jassert (type >= SourceType::Begin && type < SourceType::End);
    return type >= SourceType::Begin && type < SourceType::End 
        ? type : SourceType::Hardware;
}

//=========================================================================
void Project::getRenderContext (RenderContext& context) const
{
    context.source          = getSourceType();

    context.keyStart        = (int) getProperty (Tags::noteStart, 36);
    context.keyEnd          = (int) getProperty (Tags::noteEnd, 60);
    context.keyStride       = (int) getProperty (Tags::noteStep, 4);
    
    context.baseName        = getProperty(Tags::baseName, "Sample").toString();
    context.noteLength      = (int) getProperty (Tags::noteLength, 3000);
    context.tailLength      = (int) getProperty (Tags::tailLength, 1000);
    
    context.instrumentName  = getProperty (Tags::name, "Instrument").toString();
    context.outputPath      = getProperty(Tags::dataPath).toString();
    context.format          = getFormatTypeSlug();
    context.channels        = (int) getProperty (Tags::channels, 2);
    context.bitDepth        = (int) getProperty (Tags::bitDepth, 16);
    context.latency         = (int) getProperty (Tags::latencyComp, 0);

    // not currently used
    context.sampleRate      = 44100.0;

    for (int i = 0; i < getNumSampleSets(); ++i)
    {
        const auto layer (getSampleSet (i));
        LayerInfo info (layer.getUuidString(), layer.getVelocity());
        info.noteLength  = layer.getNoteLength();
        info.tailLength  = layer.getTailLength();
        info.midiChannel = layer.getMidiChannel();
        info.midiProgram = layer.getMidiProgram();
        context.layers.add (info);
    }
}

//=========================================================================
int Project::getNumSampleSets() const { return objectData.getChildWithName (Tags::sets).getNumChildren(); }
SampleSet Project::getSampleSet (int index) const { return SampleSet (objectData.getChildWithName (Tags::sets).getChild (index)); }

SampleSet Project::addSampleSet()
{
    auto layers = objectData.getChildWithName (Tags::sets);
    SampleSet layer;
    String layerName = "Set ";
    layerName << int (getNumSampleSets() + 1);
    layer.setProperty (Tags::velocity, 127)
         .setProperty (Tags::name, layerName);
    layers.appendChild (layer.getValueTree(), nullptr);
    return layer;
}

void Project::removeSampleSet (int index)
{
    if (! isPositiveAndBelow (index, getNumSampleSets()))
        return;
    auto layers  = objectData.getChildWithName (Tags::sets);
    auto samples = objectData.getChildWithName (Tags::samples);
    const auto layerId = layers.getChild (index).getProperty (Tags::uuid).toString();

    layers.removeChild (index, nullptr);
    for (int i = samples.getNumChildren(); --i >= 0;)
    {
        const Sample sample (samples.getChild (i));
        if (sample.getSampleSetUuidString() == layerId)
            samples.removeChild (i, nullptr);
    }
}

int Project::indexOf (const SampleSet& layer) const
{
    return objectData.getChildWithName(Tags::sets).indexOf (layer.getValueTree());
}

//=========================================================================
void Project::getAudioDeviceSetup (AudioDeviceManager::AudioDeviceSetup& setup) const
{
    setup.inputDeviceName   = getProperty (Tags::audioInput);
    setup.outputDeviceName  = getProperty (Tags::audioOutput);
    setup.bufferSize        = getProperty (Tags::bufferSize);
    setup.sampleRate        = getProperty (Tags::sampleRate);

    setup.useDefaultInputChannels   = true;
    if (auto* const data = objectData.getProperty (Tags::audioInputChannels).getBinaryData())
    {
        setup.useDefaultInputChannels = false;
        setup.inputChannels.loadFromMemoryBlock (*data);
    }

    setup.useDefaultOutputChannels  = true;
    if (auto* const data = objectData.getProperty (Tags::audioOutputChannels).getBinaryData())
    {
        setup.useDefaultOutputChannels = false;
        setup.outputChannels.loadFromMemoryBlock (*data);
    }
}

void Project::getPossibleNoteNumbers (Array<int>& notes) const
{
    int key = getProperty (Tags::noteStart, -1);
    const int step = getProperty (Tags::noteStep, -1);
    const int end  = getProperty (Tags::noteEnd, -1);
    if (key < 0 || step < 0 || end < 0)
        return;
    
    while (key <= end && key < 128)
    {
        notes.add (key);
        key += step;
    }
}

void Project::rebuildSampleList()
{
    Array<int> notes;
    getPossibleNoteNumbers (notes);
    auto samples = objectData.getOrCreateChildWithName (Tags::samples, nullptr);
    
    //objectData.removeChild (samples, nullptr);

    for (int i = 0; i < getNumSampleSets(); ++i)
    {
        const auto layer = getSampleSet (i);
        const auto layerId = layer.getUuidString();
        
        // MessageManager::getInstance()->runDispatchLoopUntil (20);

        for (const auto& note : notes)
        {
            Sample sample (find (Tags::samples, Tags::set, layerId,
                                                Tags::note,  note));
            if (! sample.isValid())
            {
                sample = Sample::create();
                sample.setProperty (Tags::note, note)
                      .setProperty (Tags::set, layerId);
                samples.appendChild (sample.getValueTree(), nullptr);
            }
        }
    }

    for (int i = samples.getNumChildren(); --i >= 0;)
    {
        const Sample sample (samples.getChild (i));
        if (! notes.contains (sample.getNote()))
            samples.removeChild (sample.getValueTree(), nullptr);
    }
}

void Project::setAudioDeviceSetup (const AudioDeviceManager::AudioDeviceSetup& setup)
{
    auto& project = *this;
    project.setProperty (Tags::audioInput, setup.inputDeviceName)
           .setProperty (Tags::audioOutput, setup.outputDeviceName)
           .setProperty (Tags::audioInputChannels, setup.inputChannels.toMemoryBlock())
           .setProperty (Tags::audioOutputChannels, setup.outputChannels.toMemoryBlock())
           .setProperty (Tags::sampleRate, setup.sampleRate)
           .setProperty (Tags::bufferSize, setup.bufferSize);
}

//=========================================================================
void Project::setSamples (const ValueTree& newSamples)
{
    for (int i = 0; i < newSamples.getNumChildren(); ++i)
    {
        const Sample recorded (newSamples.getChild (i));
        Sample existing (find (Tags::samples, Tags::set, recorded.getSampleSetUuidString(),
                                              Tags::note,  recorded.getNote()));
        
        Array<Identifier> propsToCopy, propsToCopyIfNotThere;
        propsToCopy.addArray ({ Tags::file, Tags::sampleRate, Tags::length });
        propsToCopyIfNotThere.addArray ({
            Tags::set, Tags::name, Tags::note, 
            Tags::timeIn, Tags::timeOut
        });
        
        if (! existing.isValid())
        {
            jassertfalse;
            existing = Sample::create();
            objectData.getChildWithName (Tags::samples).appendChild (
                existing.getValueTree(), nullptr);
        }

        for (const auto& prop : propsToCopy)
            existing.setProperty (prop, recorded.getProperty (prop));
        
        for (const auto& prop : propsToCopyIfNotThere)
            if (! existing.hasProperty (prop))
                existing.setProperty (prop, recorded.getProperty (prop));
        
        if (existing.getEndTime() > existing.getLength())
            existing.setProperty (Tags::timeOut, existing.getLength());
    }

   #if 0
    int lastIndex = -1;
    for (int i = objectData.getNumChildren(); --i >= 0;)
    {
        const auto samples = objectData.getChild (i);
        if (samples.hasType (Tags::samples))
        {
            lastIndex = i;
            objectData.removeChild (samples, nullptr);        
        }
    }

    objectData.addChild (newSamples, lastIndex, nullptr);
    // DBG(getSamples().getValueTree().toXmlString());
    // DBG("===================================================");
    // DBG(newSamples.toXmlString());
    // DBG("===================================================");
    // DBG(objectData.toXmlString());
   #endif
}

SampleArray Project::getSamples() const
{
    SampleArray samples (objectData.getChildWithName (Tags::samples));
    return samples;
}

void Project::getSamples (int layerIdx, OwnedArray<Sample>& out) const
{
    if (! isPositiveAndBelow (layerIdx, getNumSampleSets()))
        return;

    const auto samples = getSamples();

    const auto layer = getSampleSet (layerIdx);
    for (int i = 0; i < samples.size(); ++i)
        if (samples[i].isForSampleSet (layer))
            out.add (new Sample (samples [i]));
}

void Project::getSamplesForNote (int note, OwnedArray<Sample>& out) const
{
    const auto samples = getSamples();
    for (int i = 0; i < samples.size(); ++i)
        if (samples[i].getNote() == note)
            out.add (new Sample (samples [i]));
}

//=========================================================================
File Project::getDataPath() const
{
    const auto path = getProperty (Tags::dataPath).toString();
    return File::isAbsolutePath (path) ? File (path) : File();
}

//=========================================================================
void Project::clearPlugin()
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    if (plugin.isValid())
    {
        plugin.removeAllChildren (nullptr);
        plugin.removeAllProperties (nullptr);
    }
}

bool Project::getPluginDescription (PluginManager& plugins, PluginDescription& desc) const
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    auto& list = plugins.getKnownPlugins();
    if (const auto* const type = list.getTypeForIdentifierString (plugin.getProperty (Tags::identifier)))
    {
        desc = *type;
        return true;
    }

    return false;
}

void Project::setPluginDescription (const PluginDescription& desc)
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    plugin.setProperty (Tags::name, desc.name, nullptr)
          .setProperty (Tags::format, desc.pluginFormatName, nullptr)
          .setProperty (Tags::fileOrId, desc.fileOrIdentifier, nullptr)
          .setProperty (Tags::identifier, desc.createIdentifierString(), nullptr);
}

void Project::updatePluginState (AudioProcessor& processor)
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    MemoryBlock mb;
    processor.getStateInformation (mb);
    
    if (mb.getSize() > 0)
    {
        MemoryOutputStream mo;
        {
            GZIPCompressorOutputStream gz (mo);
            gz.write (mb.getData(), mb.getSize());
        }

        plugin.setProperty (Tags::state, mo.getMemoryBlock(), nullptr);
    }
}

void Project::applyPluginState (AudioProcessor& processor) const
{
    const auto plugin = objectData.getChildWithName (Tags::plugin);
    if (const auto* state = plugin.getProperty(Tags::state).getBinaryData())
    {
        MemoryInputStream mi (*state, false);
        GZIPDecompressorInputStream gz (mi);
        MemoryBlock mb;
        gz.readIntoMemoryBlock (mb);
        processor.setStateInformation (mb.getData(), static_cast<int> (mb.getSize()));
    }
}

//=========================================================================
bool Project::writeToFile (const File& file) const
{
    TemporaryFile tempFile (file);
    auto dataCopy = objectData.createCopy();
    auto expCopy = dataCopy.getChildWithName(Tags::exporters);
    for (int i = 0; i < expCopy.getNumChildren(); ++i)
        expCopy.getChild(i).removeProperty (Tags::object, nullptr);

    {
        FileOutputStream fo (tempFile.getFile());
        {
            GZIPCompressorOutputStream go (fo);
            dataCopy.writeToStream (go);
        }
    }

    return tempFile.overwriteTargetFileWithTemporary();
}

bool Project::loadFile (const File& file)
{
    if (! file.existsAsFile())
        return false;
    
    FileInputStream fi (file);
    GZIPDecompressorInputStream gi (fi);
    auto newData = ValueTree::readFromStream (gi);
    if (newData.isValid() && newData.hasType (Tags::project))
    {
        objectData = newData;
        setMissingProperties();
    }

    return objectData == newData;
}

void Project::setMissingProperties()
{
    stabilizePropertyPOD (Tags::version, (int) VCP_PROJECT_FILE_VERSION);
    stabilizePropertyString (Tags::name, {});
    stabilizePropertyString (Tags::dataPath, {});
    
    RenderContext context;
    stabilizePropertyString (Tags::source,      SourceType::getSlug (SourceType::Hardware));
    stabilizePropertyPOD (Tags::latencyComp,    0);
    stabilizePropertyPOD (Tags::noteStart,      36);
    stabilizePropertyPOD (Tags::noteEnd,        60);
    stabilizePropertyPOD (Tags::noteStep,       4);
    
    stabilizePropertyString (Tags::baseName,    "Sample");
    stabilizePropertyPOD (Tags::noteLength,     3000);
    stabilizePropertyPOD (Tags::tailLength,     1000);

    stabilizePropertyString (Tags::format,      FormatType::getSlug (FormatType::WAVE));
    stabilizePropertyPOD (Tags::channels,       context.channels);
    stabilizePropertyPOD (Tags::bitDepth,       context.bitDepth);

    objectData.getOrCreateChildWithName (Tags::exporters,  nullptr);
    objectData.getOrCreateChildWithName (Tags::sets,  nullptr);
    objectData.getOrCreateChildWithName (Tags::samples, nullptr);
    objectData.getOrCreateChildWithName (Tags::plugin,  nullptr);
}

ValueTree Project::find (const Identifier& listType, 
                         const Identifier& property,
                         const var& value) const
{
    const auto parent = objectData.getChildWithName (listType);
    return parent.getChildWithProperty (property, value);
}

ValueTree Project::find (const Identifier& listType,
                         const Identifier& p1, const var& v1,
                         const Identifier& p2, const var& v2) const
{
    const auto parent = objectData.getChildWithName (listType);
    for (int i = 0; i < parent.getNumChildren(); ++i)
        if (parent.getChild (i)[p1] == v1 && parent.getChild(i)[p2] == v2)
            return parent.getChild (i);
    return { };
}

void Project::addExporter (ExporterType& type, const String& name)
{
    auto exporters = objectData.getChildWithName (Tags::exporters);
    ValueTree data = type.createModelData();
    if (name.isNotEmpty())
        data.setProperty (Tags::name, name, nullptr);
    auto path = getDataPath();
    
    if (path != File())
    {
        path = path.getChildFile("export");
        path = path.getNonexistentChildFile (type.getSlug().toUpperCase(), " ", false);
        data.setProperty (Tags::path, path.getFullPathName(), nullptr);
    }

    exporters.appendChild (data, nullptr);
}

void Project::setActiveExporter (int index)
{
    auto tree = getExportersTree();
    auto exporter = tree.getChild (index);
    if (exporter.hasType (Tags::exporter))
        tree.setProperty (Tags::active, exporter.getProperty (Tags::uuid), nullptr);
}

ValueTree Project::getActiveExporterData() const
{
    const auto tree = getExportersTree();
    const auto& uuid = tree.getProperty (Tags::active);
    if (uuid.toString().isEmpty())
        return ValueTree();
    return tree.getChildWithProperty (Tags::uuid, uuid);
}

}

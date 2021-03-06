#pragma once

#include "Project.h"

namespace vcp {

class ProjectWatcher : private ValueTree::Listener
{
public:
    ProjectWatcher() = default;
    virtual ~ProjectWatcher() {}
    
    class ScopedBlock
    {
    public:
        ScopedBlock (ProjectWatcher& w)
            : watcher (w) { watcher.blocked = true; }
        ~ScopedBlock() { watcher.blocked = false; }
    private:
        ProjectWatcher& watcher;
    };

    void setProject (const Project& newProject)
    {
        if (data == newProject.getValueTree())
            return;
        
        data.removeListener (this);
        project = newProject;
        data = project.getValueTree();
        data.addListener (this);
        if (onChanged)
            onChanged();
    }

    Project getProject() const { return project; }

    std::function<void()> onChanged;
    std::function<void()> onSourceChanged;
    std::function<void()> onLayerAdded;
    std::function<void()> onLayerRemoved;
    std::function<void()> onActiveLayerChanged;

    std::function<void()> onSamplesRemoved;
    std::function<void()> onSamplesAdded;
    std::function<void()> onSampleRemoved;
    std::function<void()> onSampleAdded;
    std::function<void()> onActiveSampleChanged;

    std::function<void()> onExportersChanged;
    std::function<void()> onActiveExporterChanged;

    std::function<void()> onProjectModified;

private:
    bool blocked = false;
    Project project;
    ValueTree data;

    void notifyModified()
    {
        if (onProjectModified)
            onProjectModified();
    }

    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property) override
    {
        if (blocked)
            return;

        if (tree.hasType (Tags::project) && property == Tags::source)
        {
            if (onSourceChanged)
                onSourceChanged();
        }
        else if (tree.hasType (Tags::sets) && property == Tags::active)
        {
            if (onActiveLayerChanged)
                onActiveLayerChanged();
        }
        else if (tree.hasType (Tags::exporters) && property == Tags::active)
        {
            if (onActiveExporterChanged)
                onActiveExporterChanged();
        }
        else if (tree.hasType (Tags::samples) && property == Tags::active)
        {
            if (onActiveSampleChanged)
                onActiveSampleChanged();
        }

        if (tree.hasType (Tags::project))
            notifyModified();
    }

    void valueTreeChildAdded (ValueTree& parent, ValueTree& child) override
    {
        if (blocked) return;

        if (child.hasType (Tags::set) && parent.getParent() == data)
        {
            if (onLayerAdded)
                onLayerAdded();
        }
        else if (child.hasType (Tags::samples) && parent == data)
        {
            if (onSamplesAdded)
                onSamplesAdded();
        }
        else if (child.hasType (Tags::sample) && 
                 parent.hasType (Tags::samples) && 
                 parent.getParent() == data)
        {
            if (onSampleAdded)
                onSampleAdded();
        }
        else if (child.hasType (Tags::exporter) && parent.hasType (Tags::exporters))
        {
            if (onExportersChanged)
                onExportersChanged();
        }

        notifyModified();
    }

    void valueTreeChildRemoved (ValueTree& parent, ValueTree& child, int index) override
    {
        if (blocked) return;

        if (child.hasType (Tags::set) && parent.getParent() == data)
        {
            if (onLayerRemoved)
                onLayerRemoved();
        }
        else if (child.hasType (Tags::samples) && parent == data)
        {
            if (onSamplesRemoved)
                onSamplesRemoved();
        }
        else if (child.hasType (Tags::sample) && 
                 parent.hasType (Tags::samples) && 
                 parent.getParent() == data)
        {
            if (onSampleRemoved)
                onSampleRemoved();
        }
        else if (child.hasType (Tags::exporter) && parent.hasType (Tags::exporters))
        {
            if (onExportersChanged)
                onExportersChanged();
        }
        
        notifyModified();
    }

    void valueTreeChildOrderChanged (ValueTree& parent, int oldIndex, int newIndex) override
    {
        if (blocked) return;
        ignoreUnused (parent, oldIndex, newIndex);
        notifyModified();
    }

    void valueTreeParentChanged (ValueTree& tree) override
    {
        if (blocked) return;
        ignoreUnused (tree);
    }

    void valueTreeRedirected (ValueTree& tree) override
    {
        if (blocked) return;
        ignoreUnused (tree);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectWatcher)
};

}

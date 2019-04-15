#pragma once

#include "Project.h"

namespace vcp {

class ProjectWatcher : private ValueTree::Listener
{
public:
    ProjectWatcher() = default;
    virtual ~ProjectWatcher() {}

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
    std::function<void()> onLayerAdded;
    std::function<void()> onLayerRemoved;
    std::function<void()> onActiveLayerChanged;

    std::function<void()> onSamplesRemoved;
    std::function<void()> onSamplesAdded;
    std::function<void()> onActiveSampleChanged;

private:
    Project project;
    ValueTree data;

    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property) override
    {
        if (tree.hasType (Tags::layers) && property == Tags::active)
        {
            if (onActiveLayerChanged)
                onActiveLayerChanged();
        }
        else if (tree.hasType (Tags::samples) && property == Tags::active)
        {
            if (onActiveSampleChanged)
                onActiveSampleChanged();
        }
    }

    void valueTreeChildAdded (ValueTree& parent, ValueTree& child) override
    {
        if (child.hasType (Tags::layer) && parent.getParent() == data)
        {
            if (onLayerAdded)
                onLayerAdded();
        }
        else if (child.hasType (Tags::samples) && parent == data)
        {
            if (onSamplesAdded)
                onSamplesAdded();
        }
    }

    void valueTreeChildRemoved (ValueTree& parent, ValueTree& child, int index) override
    {
        if (child.hasType (Tags::layer) && parent.getParent() == data)
        {
            if (onLayerRemoved)
                onLayerRemoved();
        }
        else if (child.hasType (Tags::samples) && parent == data)
        {
            if (onSamplesRemoved)
                onSamplesRemoved();
        }
    }

    void valueTreeChildOrderChanged (ValueTree& parent, int oldIndex, int newIndex) override
    {
        ignoreUnused (parent, oldIndex, newIndex);
    }

    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged) override
    {
        ignoreUnused (treeWhoseParentHasChanged);
    }

    void valueTreeRedirected (ValueTree& tree) override
    {
        ignoreUnused (tree);
    }
};

}

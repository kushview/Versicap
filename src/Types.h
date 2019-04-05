#pragma once

#include "JuceHeader.h"

namespace vcp {

struct LoopType
{
    enum ID
    {
        None = 0,
        Forwards,
        Alternating,
        Reverse,
        RoundRobin
    };
    
    enum
    {
        NumTypes = 5,
        Begin = 0,
        End = NumTypes
    };

    inline String getName() const { return getName (type); }
    inline static String getName (int t)
    {
        switch (t)
        {
            case None:          return "None"; break;
            case Forwards:      return "Forwards"; break;
            case Alternating:   return "Alertnating"; break;
            case Reverse:       return "Reverse"; break;
            case RoundRobin:    return "Round Robin"; break;
        }
        return "Unknown";
    }

    inline String getSlug() const { return getSlug (type); }
    inline static String getSlug (int t)
    {
        switch (t)
        {
            case None:          return "none"; break;
            case Forwards:      return "forwards"; break;
            case Alternating:   return "alertnating"; break;
            case Reverse:       return "reverse"; break;
            case RoundRobin:    return "roundRobin"; break;
        }

        return "Unknown";
    }

    LoopType() = default;
    LoopType (const int t) : type (t) { jassert (isPositiveAndBelow (t, NumTypes)); }
    LoopType (const ID t) : type (static_cast<int> (t)) {}
    LoopType (const LoopType& o) { operator= (o); }
    LoopType& operator= (const LoopType& o) { type = o.type; return *this; }
    LoopType& operator= (const int t) { type = t; jassert (isPositiveAndBelow (t, NumTypes)); return *this; }
    LoopType& operator= (const ID t) { type = static_cast<int> (t); return *this; }

    bool operator== (const int t) const { return t == type; }
    bool operator!= (const int t) const { return t != type; }
    bool operator== (const ID t)  const { return t == type; }
    bool operator!= (const ID t)  const { return t != type; }

private:
    int type = 0;
};

struct SourceType
{
    enum ID
    {
        MidiDevice  = 0,
        AudioPlugin = 1
    };
};

}

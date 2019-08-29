#pragma once
#include <juce/juce.h>

namespace juce {}
using namespace juce;

#include "../../jucer/JuceLibraryCode/BinaryData.h"
namespace kv {}
using namespace kv;

#include "../../libs/kv/modules/kv_core/kv_core.h"
#include "../../libs/kv/modules/kv_edd/kv_edd.h"
#include "../../libs/kv/modules/kv_gui/kv_gui.h"
#include "../../libs/kv/modules/kv_models/kv_models.h"
#if HAVE_LILV && HAVE_SUIL && HAVE_LV2
 #include "../../libs/jlv2/modules/jlv2_host/jlv2_host.h"
#endif

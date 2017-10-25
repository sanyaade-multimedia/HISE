
#include "hi_streaming.h"

#include "hi_streaming/lockfree_fifo/readerwriterqueue.h"

namespace hise
{
using namespace juce;

#include "hi_streaming/SampleThreadPool.cpp"
#include "hi_streaming/MonolithAudioFormat.cpp"
#include "hi_streaming/StreamingSampler.cpp"
#include "hi_streaming/StreamingSamplerSound.cpp"
#include "hi_streaming/StreamingSamplerVoice.cpp"

}




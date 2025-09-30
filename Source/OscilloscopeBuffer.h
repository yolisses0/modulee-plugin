#pragma once
#include <JuceHeader.h>

class OscilloscopeBuffer {
public:
  OscilloscopeBuffer(int size)
      : buffer(), accumulator(0.0f), ratio(1.0f), tail(0) {
    buffer.setSize(1, size);
    buffer.clear();
  }

  void push(float value) {
    accumulator += ratio;
    while (accumulator >= 1.0f) {
      buffer.setSample(0, tail, value);
      tail = (tail + 1) % buffer.getNumSamples();
      accumulator -= 1.0f;
    }
  }

  void setRatio(float newRatio) { ratio = newRatio; }

  juce::var getDataVar() { return getVarFromAudioBuffer(buffer); }

private:
  juce::AudioBuffer<float> buffer;
  float accumulator;
  float ratio;
  int tail;

  juce::var getVarFromAudioBuffer(const juce::AudioBuffer<float> &buffer) {
    int numSamples = buffer.getNumSamples();
    int channel = 0;
    const float *channelData = buffer.getReadPointer(channel);

    juce::Array<juce::var> sampleArray;
    sampleArray.ensureStorageAllocated(numSamples);

    for (int i = 0; i < numSamples; ++i) {
      sampleArray.add(juce::var(channelData[i]));
    }

    return juce::var(sampleArray);
  }
};
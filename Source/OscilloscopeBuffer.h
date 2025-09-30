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

  const juce::AudioBuffer<float> &getBuffer() const { return buffer; }

private:
  juce::AudioBuffer<float> buffer;
  float accumulator;
  float ratio;
  int tail;
};
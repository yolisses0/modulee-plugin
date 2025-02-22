/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../Vendor/modulee-engine-clib/pkg/bindings.h"
#include <JuceHeader.h>

//==============================================================================
/**
 */
class ModuleeAudioProcessor : public juce::AudioProcessor {
public:
  void setGraph(const char *graph_data);

  //==============================================================================
  ModuleeAudioProcessor();
  ~ModuleeAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  std::mutex graph_mutex;

  /* `unique_ptr` is a smart pointer that will call the destructor once there's
  no reference to this. This line is just a declaration though. The real
  initialization value is given in the plugin processor constructor. */
  std::unique_ptr<Graph, decltype(&destroy_graph_pointer)> graph;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleeAudioProcessor)
};

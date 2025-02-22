/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "ProjectManager.h"
#include <JuceHeader.h>

//==============================================================================
/**
 */
class ModuleeAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  ModuleeAudioProcessorEditor(ModuleeAudioProcessor &);
  ~ModuleeAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  ProjectManager projectManager;

  void handleSetGraph(juce::var data);

  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  ModuleeAudioProcessor &audioProcessor;

  std::unique_ptr<juce::WebBrowserComponent> webView;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleeAudioProcessorEditor)
};

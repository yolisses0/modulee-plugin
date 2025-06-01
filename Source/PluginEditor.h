/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "AuthTokenManager.h"
#include "OAuthServer.h"
#include "PluginProcessor.h"
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
  juce::WebBrowserComponent::Options getWebviewOptions();

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  ModuleeAudioProcessor &audioProcessor;

  AuthTokenManager tokenManager;
  std::unique_ptr<OAuthServer> server;
  std::unique_ptr<juce::WebBrowserComponent> webView;

  void startServer();
  void stopServer();
  void openUrl(juce::String url);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleeAudioProcessorEditor)
};

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

  bool keyStateChanged(bool isKeyDown) override;
  bool keyPressed(const juce::KeyPress &key) override;

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

  int getPitch(char key) const;

  std::map<char, int> keyToNoteMap = {
      {'a', 60}, // C4
      {'s', 62}, // D4
      {'d', 64}, // E4
      {'f', 65}, // F4
      {'g', 67}, // G4
      {'h', 69}, // A4
      {'j', 71}, // B4
      {'k', 72}, // C5
      {'l', 74}  // D5
  };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleeAudioProcessorEditor)
};

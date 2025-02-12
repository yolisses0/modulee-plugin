/*
  ==============================================================================

        This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <iostream>

//==============================================================================
ModuleeAudioProcessorEditor::ModuleeAudioProcessorEditor(
    ModuleeAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {

  // this may be necessary for some DAWs; include for safety
  auto userDataFolder = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::tempDirectory);

  // configure Windows WebView2
  auto winWebViewOptions = juce::WebBrowserComponent::Options::WinWebView2{}
                               .withBackgroundColour(juce::Colour(0xff18181b))
                               .withUserDataFolder(userDataFolder);

  std::cout << "cout is working" << std::endl;
  auto webBrowserOptions =
      juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(winWebViewOptions)
          .withEventListener("setGroups",
                             [this](juce::var data) { handleSetGroups(data); })
          .withEventListener(
              "setMainGroupId",
              [this](juce::var data) { handleSetMainGroupId(data); })
          .withNativeIntegrationEnabled();

  auto webBrowserComponent = new juce::WebBrowserComponent(webBrowserOptions);

  webView.reset(webBrowserComponent);
  addAndMakeVisible(webView.get());

  // Determine the URL based on the build configuration
#ifdef IS_DEV_ENVIRONMENT
  webView->goToURL("http://localhost:5173");
#else
  webView->goToURL("https://modulee.yolisses.com");
#endif

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(600, 400);
  setResizable(true, true);
}

void ModuleeAudioProcessorEditor::handleSetGroups(juce::var data) {
  auto groups_data = data.getProperty("groupsData", "").toString();
  audioProcessor.setGroups(groups_data.toStdString().c_str());
}

void ModuleeAudioProcessorEditor::handleSetMainGroupId(juce::var data) {
  auto mainGroupId = (uint64_t)data.getProperty("mainGroupId", "")
                         .toString()
                         .getLargeIntValue();
  DBG("handleSetMainGroupId" << mainGroupId);
  audioProcessor.setMainGroupId(mainGroupId);
}

ModuleeAudioProcessorEditor::~ModuleeAudioProcessorEditor() {}

//==============================================================================
void ModuleeAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ModuleeAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  webView->setBounds(0, 0, getWidth(), getHeight());
}
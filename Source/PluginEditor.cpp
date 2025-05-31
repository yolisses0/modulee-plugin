/*
  ==============================================================================

        This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "OAuthServer.h"
#include "PluginProcessor.h"
#include <iostream>

//==============================================================================
ModuleeAudioProcessorEditor::ModuleeAudioProcessorEditor(
    ModuleeAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), server(), tokenManager() {

  auto webBrowserOptions = getWebviewOptions();
  auto webBrowserComponent = new juce::WebBrowserComponent(webBrowserOptions);
  webView.reset(webBrowserComponent);
  addAndMakeVisible(webView.get());

  // Determine the URL based on the build configuration and last path accessed
#ifdef IS_DEV_ENVIRONMENT
  auto url = juce::String("http://localhost:5173");
#else
  auto url = juce::String("https://modulee.yolisses.com");
#endif
  url += audioProcessor.lastPath;
  webView->goToURL(url);

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(800, 600);
  setResizable(true, true);

  startServer();
  server->onCodeReceived = [this, url](const std::string &code) {
    juce::MessageManager::callAsync([this, url, code] {
      webView->emitEventIfBrowserIsVisible("signInResponse",
                                           juce::String(code));
    });
  };
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

void ModuleeAudioProcessorEditor::startServer() {
  if (!server) {
    server = std::make_unique<OAuthServer>();
    server->startThread();
    juce::Logger::writeToLog(
        "Server started. Try accessing http://localhost:8080");
  }
}

void ModuleeAudioProcessorEditor::stopServer() {
  if (server) {
    server->stop();
    server.reset();
    juce::Logger::writeToLog("Server stopped.");
  }
}

juce::WebBrowserComponent::Options
ModuleeAudioProcessorEditor::getWebviewOptions() {

  // this may be necessary for some DAWs; include for safety
  auto userDataFolder = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::tempDirectory);

  auto winWebViewOptions = juce::WebBrowserComponent::Options::WinWebView2{}
                               .withBackgroundColour(juce::Colour(0xff18181b))
                               .withUserDataFolder(userDataFolder);

  auto webBrowserOptions =
      juce::WebBrowserComponent::Options{}
          .withNativeIntegrationEnabled()
          .withWinWebView2Options(winWebViewOptions)
          .withInitialisationData("isRunningOnJucePlugin", true)
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          //  Open in browser
          .withEventListener("openUrl",
                             [this](auto data) {
                               juce::String url =
                                   data.getProperty("url", juce::var());
                               openUrl(url);
                             })
          //  Page persistence
          .withEventListener("setPath",
                             [this](auto data) {
                               juce::String path =
                                   data.getProperty("path", juce::var());
                               audioProcessor.lastPath = path;
                             })
          //  Audio backend
          .withEventListener("setNoteOn",
                             [this](auto data) {
                               int pitch =
                                   data.getProperty("pitch", juce::var());
                               audioProcessor.setNoteOn(pitch);
                             })
          .withEventListener("setNoteOff",
                             [this](auto data) {
                               int pitch =
                                   data.getProperty("pitch", juce::var());
                               audioProcessor.setNoteOff(pitch);
                             })
          .withEventListener("setGraph",
                             [this](auto data) {
                               juce::String graphEngineData = data.getProperty(
                                   "graphEngineData", juce::var());
                               audioProcessor.setGraph(graphEngineData);
                             })
          .withEventListener("setIsMuted",
                             [this](juce::var data) {
                               bool isMuted =
                                   data.getProperty("isMuted", juce::var());
                               audioProcessor.isMuted = isMuted;
                             })
          .withEventListener("updateControl", [this](juce::var data) {
            juce::int64 id = data.getProperty("id", juce::var());
            float value = data.getProperty("value", juce::var());
            audioProcessor.updateControl(id, value);
          });

  return webBrowserOptions;
}

void ModuleeAudioProcessorEditor::openUrl(juce::String url) {
  juce::URL(url).launchInDefaultBrowser();
}
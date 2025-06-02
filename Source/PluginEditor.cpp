/*
  ==============================================================================

        This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "OAuthServer.h"
#include "PluginProcessor.h"
#include <iostream>

const auto SESSION_COOKIE_NAME = juce::String("session");
const auto SET_AUTH_TOKEN_ROUTE = juce::String("/setAuthToken");

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

  auto authToken = tokenManager.getAuthToken();
  if (authToken.has_value()) {
    url += "/setAuthToken";
  } else {
    url += audioProcessor.lastPath;
  }
  webView->goToURL(url);

  // Make sure that before the constructor has finished, you've set the editor's
  // size to whatever you need it to be.
  setSize(800, 600);
  setResizable(true, true);
  setWantsKeyboardFocus(true);

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
          .withInitialisationData("lastPage", audioProcessor.lastPath)
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          // Auth token
          .withEventListener("setAuthToken",
                             [this](auto data) {
                               juce::String authToken =
                                   data.getProperty("authToken", juce::var());
                               tokenManager.setAuthToken(authToken);
                             })
          // Open in browser
          .withEventListener("openUrl",
                             [this](auto data) {
                               juce::String url =
                                   data.getProperty("url", juce::var());
                               openUrl(url);
                             })
          // Page persistence
          .withEventListener("setPath",
                             [this](auto data) {
                               juce::String path =
                                   data.getProperty("path", juce::var());
                               audioProcessor.lastPath = path;
                             })
          // Audio backend
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

  // Auth token
  auto authToken = tokenManager.getAuthToken();
  if (authToken.has_value()) {
    webBrowserOptions = webBrowserOptions.withInitialisationData(
        "authToken", authToken.value());
  }

  return webBrowserOptions;
}

void ModuleeAudioProcessorEditor::openUrl(juce::String url) {
  juce::URL(url).launchInDefaultBrowser();
}

int ModuleeAudioProcessorEditor::getPitch(char key) const {
  key = std::tolower(key);
  // TODO make more intuitive
  auto it = keyToNoteMap.find(key);
  return it != keyToNoteMap.end() ? it->second : -1;
}

bool ModuleeAudioProcessorEditor::keyPressed(const juce::KeyPress &key) {
  int pitch = getPitch(key.getTextCharacter());
  if (pitch != -1) {
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, pitch, 0.8f);
    audioProcessor.setNoteOn(pitch);

    return true;
  }
  return false;
}

bool ModuleeAudioProcessorEditor::keyStateChanged(bool isKeyDown) {
  if (!isKeyDown) {
    for (auto &pair : keyToNoteMap) {
      if (!juce::KeyPress::isKeyCurrentlyDown(pair.first)) {
        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, pair.second);
        audioProcessor.setNoteOff(pair.second);
      }
    }
  }
  return false;
}

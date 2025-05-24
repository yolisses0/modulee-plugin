/*
  ==============================================================================

        This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "OAuthServer.h"
#include "PluginProcessor.h"
#include "ProjectRepository.h"
#include <iostream>

//==============================================================================
ModuleeAudioProcessorEditor::ModuleeAudioProcessorEditor(
    ModuleeAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), projectRepository(),
      server() {

  // this may be necessary for some DAWs; include for safety
  auto userDataFolder = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::tempDirectory);

  // configure Windows WebView2
  auto winWebViewOptions = juce::WebBrowserComponent::Options::WinWebView2{}
                               .withBackgroundColour(juce::Colour(0xff18181b))
                               .withUserDataFolder(userDataFolder);

  auto webBrowserOptions =
      juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(winWebViewOptions)
          .withInitialisationData("isRunningOnJucePlugin", true)
          .withNativeIntegrationEnabled()
          //  Page persistence
          .withEventListener("setPath",
                             [this](auto data) {
                               auto path =
                                   data.getProperty("path", false).toString();
                               audioProcessor.lastPath = path;
                             })
          //  Audio backend
          .withEventListener("setNoteOn",
                             [this](auto data) {
                               auto pitch =
                                   (int)data.getProperty("pitch", juce::var());
                               audioProcessor.setNoteOn(pitch);
                             })
          .withEventListener("setNoteOff",
                             [this](auto data) {
                               auto pitch =
                                   (int)data.getProperty("pitch", juce::var());
                               audioProcessor.setNoteOff(pitch);
                             })
          .withEventListener(
              "setGraph",
              [this](auto data) {
                auto graphEngineData =
                    data.getProperty("graphEngineData", juce::var()).toString();
                audioProcessor.setGraph(graphEngineData);
              })
          .withEventListener("setIsMuted",
                             [this](juce::var data) {
                               bool isMuted =
                                   data.getProperty("isMuted", false);
                               audioProcessor.isMuted = isMuted;
                             })
          //  Projects repository
          .withNativeFunction("getProjects",
                              [this](auto &args, auto completion) {
                                auto projectsJson =
                                    projectRepository.getProjects();
                                completion(projectsJson);
                              })
          .withNativeFunction("getProject",
                              [this](auto &args, auto completion) {
                                auto projectJson =
                                    projectRepository.getProject(args[0]);
                                completion(projectJson);
                              })
          .withNativeFunction("createProject",
                              [this](auto &args, auto completion) {
                                projectRepository.createProject(args[0]);
                                completion(true);
                              })
          .withNativeFunction("deleteProject",
                              [this](auto &args, auto completion) {
                                projectRepository.deleteProject(args[0]);
                                completion(true);
                              })
          .withNativeFunction("renameProject",
                              [this](auto &args, auto completion) {
                                projectRepository.renameProject(args[0],
                                                                args[1]);
                                completion(true);
                              })
          .withNativeFunction(
              "updateProjectGraph", [this](auto &args, auto completion) {
                projectRepository.updateProjectGraph(args[0], args[1]);
                completion(true);
              });

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

  // juce::String clientId =
  // "725523345294-l7ljv04v2maac7k6ugu6ifmuut88gbjk.apps."
  //                         "googleusercontent.com";
  // // TODO replace by custom URI like "com.myapp://oauth"
  // juce::String redirectUri = "http://localhost:5173/signIn/response";
  // juce::String scope = "https://www.googleapis.com/auth/userinfo.profile "
  //                      "https://www.googleapis.com/auth/userinfo.email";
  // juce::String authUrl = "https://accounts.google.com/o/oauth2/v2/auth?"
  //                        "client_id=" +
  //                        clientId + "&redirect_uri=" + redirectUri +
  //                        "&response_type=code"
  //                        "&scope=" +
  //                        scope + "&access_type=offline";
  // juce::URL(authUrl).launchInDefaultBrowser();
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
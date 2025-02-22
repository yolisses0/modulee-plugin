/*
  ==============================================================================

        This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ProjectManager.h"
#include <iostream>

//==============================================================================
ModuleeAudioProcessorEditor::ModuleeAudioProcessorEditor(
    ModuleeAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p), projectManager() {

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
          .withEventListener("setGraph",
                             [this](juce::var data) { handleSetGraph(data); })
          .withNativeFunction("getProjects",
                              [this](auto &args, auto completion) {
                                auto projectsJson =
                                    projectManager.getProjects();
                                completion(projectsJson);
                              })
          .withNativeFunction("getProject",
                              [this](auto &args, auto completion) {
                                auto projectJson =
                                    projectManager.getProject(args[0]);
                                completion(projectJson);
                              })
          .withNativeFunction("createProject",
                              [this](auto &args, auto completion) {
                                projectManager.createProject(args[0]);
                                completion(true);
                              })
          .withNativeFunction("deleteProject",
                              [this](auto &args, auto completion) {
                                projectManager.deleteProject(args[0]);
                                completion(true);
                              })
          .withNativeFunction("addCommand",
                              [this](auto &args, auto completion) {
                                projectManager.addCommand(args[0]);
                                completion(true);
                              })
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

void ModuleeAudioProcessorEditor::handleSetGraph(juce::var data) {
  auto graph_data = data.getProperty("graphData", "").toString();
  audioProcessor.setGraph(graph_data.toStdString().c_str());
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

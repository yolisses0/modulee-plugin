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

  auto webBrowserOptions =
      juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(winWebViewOptions)
          .withEventListener("setGraph",
                             [this](juce::var data) { handleSetGraph(data); })
          .withNativeFunction("getSavedData",
                              [this](auto &args, auto completion) {
                                getSavedData(args, std::move(completion));
                              })
          .withNativeFunction("setSavedData",
                              [this](auto &args, auto completion) {
                                setSavedData(args, std::move(completion));
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

juce::File ModuleeAudioProcessorEditor::getSavedDataFile() {
  auto savedDataFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile("Modulee/savedData.json");

  auto isFolderCreated = savedDataFile.createDirectory();

  if (isFolderCreated.failed()) {
    DBG("Failed to create the parent folder");
  }

  return savedDataFile;
}

void ModuleeAudioProcessorEditor::setSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {
  DBG("setSavedData start");

  // Check if the input argument is valid
  if (args.size() > 0 && args[0].isString()) {
    juce::String dataToSave = args[0].toString();

    juce::File savedDataFile = getSavedDataFile();

    // Save the string to the file
    if (savedDataFile.replaceWithText(dataToSave)) {
      DBG("Data saved to file: " << savedDataFile.getFullPathName());
      completion(true); // Indicate success to the JavaScript caller
    } else {
      DBG("Failed to save data to file!");
      completion(false); // Indicate failure to the JavaScript caller
    }
  } else {
    DBG("Invalid argument: Expected a string!");
    completion(false); // Indicate failure to the JavaScript caller
  }

  DBG("setSavedData end");
}

void ModuleeAudioProcessorEditor::getSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {
  DBG("getSavedData start");

  auto savedDataFile = getSavedDataFile();
  // Use the constant file path
  if (savedDataFile.existsAsFile()) {
    // Load the string from the file
    juce::String loadedData = savedDataFile.loadFileAsString();
    DBG("Data loaded from file: " << loadedData);
    completion(
        loadedData); // Pass the loaded data back to the JavaScript caller
  } else {
    DBG("File does not exist!");
    completion(juce::var()); // Return an empty value to indicate failure
  }

  DBG("getSavedData end");
}
#pragma once

#include <JuceHeader.h>

class ProjectManager {
public:
  ProjectManager();

  // void initialize();
  // bool getIsInitialized();
  juce::String getProjects();
  // void deleteProject(juce::String id);
  juce::String getProject(juce::String id);
  // void addCommand(juce::String commandDataJson);
  // void createProject(juce::String projectDataJson);

  //   DEBUG
  void
  setSavedData(const juce::Array<juce::var> &args,
               juce::WebBrowserComponent::NativeFunctionCompletion completion);
  void
  getSavedData(const juce::Array<juce::var> &args,
               juce::WebBrowserComponent::NativeFunctionCompletion completion);

private:
  juce::File getSavedDataFile();
  juce::String projectsDirectoryPath;
};
#pragma once

#include <JuceHeader.h>

class ProjectManager {
public:
  ProjectManager();
  juce::String getProjects();
  void deleteProject(juce::String id);
  juce::String getProject(juce::String id);
  void createProject(juce::String projectDataJson);
  void renameProject(juce::String id, juce::String name);

private:
  juce::String projectsDirectoryPath;
};
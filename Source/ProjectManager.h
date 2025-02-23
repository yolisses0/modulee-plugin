#pragma once

#include <JuceHeader.h>

class ProjectManager {
public:
  ProjectManager();
  juce::String getProjects();
  void deleteProject(juce::String id);
  juce::String getProject(juce::String id);
  void addCommand(juce::String commandDataJson);
  void createProject(juce::String projectDataJson);
  void renameProject(juce::String id, juce::String name);

private:
  // Change this attribute name to force a cache invalidation
  int test_1;
  juce::String projectsDirectoryPath;
  juce::var getProjectCommands(juce::String projectId);
};
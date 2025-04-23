#pragma once

#include <JuceHeader.h>

class ProjectRepository {
public:
  ProjectRepository();
  juce::String getProjects();
  void deleteProject(juce::String id);
  juce::String getProject(juce::String id);
  void createProject(juce::String projectDataJson);
  void renameProject(juce::String id, juce::String name);
  void updateProjectGraph(juce::String id, juce::String graphData);

private:
  juce::String projectsDirectoryPath;
  juce::File getProjectFile(juce::String id);
  juce::File createProjectFile(juce::String id);
  juce::String getProjectFilePath(juce::String projectId);
};

#include "ProjectRepository.h"

// TODO replace projectId/index.json by projectId.json

// TODO consider replacing this manual files manipulation by some proper
// database like SQLite

ProjectRepository::ProjectRepository() {
  projectsDirectoryPath = "Modulee/projects";
}

juce::String ProjectRepository::getProjectFilePath(juce::String id) {
  auto projectFilePath = projectsDirectoryPath + "/" + id + ".json";
  return projectFilePath;
}

juce::File ProjectRepository::getProjectFile(juce::String id) {
  auto projectFilePath = getProjectFilePath(id);
  auto projectFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectFilePath);
  return projectFile;
}

juce::File ProjectRepository::createProjectFile(juce::String id) {
  auto projectFile = getProjectFile(id);
  projectFile.getParentDirectory().createDirectory();
  return projectFile;
}

juce::String ProjectRepository::getProjects() {
  auto folder =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectsDirectoryPath);
  folder.createDirectory();

  juce::Array<juce::File> files;
  folder.findChildFiles(files, juce::File::findFiles, false);

  juce::var jsonArray = juce::var(juce::Array<juce::var>{});

  for (const auto &file : files) {
    auto fileJson = juce::JSON::parse(file);
    jsonArray.append(fileJson);
  }

  return juce::JSON::toString(jsonArray);
}

void ProjectRepository::deleteProject(juce::String id) {
  auto projectFile = getProjectFile(id);
  projectFile.deleteFile();
}

juce::String ProjectRepository::getProject(juce::String id) {
  auto projectFile = getProjectFile(id);
  return projectFile.loadFileAsString();
}

void ProjectRepository::createProject(juce::String projectDataJson) {
  auto projectData = juce::JSON::parse(projectDataJson);
  auto id = projectData.getProperty("id", false).toString();
  auto projectFile = createProjectFile(id);

  // These parse and toString probably aren't needed. It's here just to ensure
  // consistent encoding during development.
  auto data = juce::JSON::toString(projectData);
  projectFile.replaceWithData(data.getCharPointer(), data.getNumBytesAsUTF8());
}

void ProjectRepository::renameProject(juce::String id, juce::String name) {
  auto projectFile = getProjectFile(id);
  auto projectData = juce::JSON::parse(projectFile);

  projectData.getDynamicObject()->setProperty("name", name);

  auto projectDataString = juce::JSON::toString(projectData);
  projectFile.replaceWithData(projectDataString.getCharPointer(),
                              projectDataString.getNumBytesAsUTF8());
}

void ProjectRepository::updateProjectGraph(juce::String id,
                                           juce::String graphDataJson) {
  auto projectFile = getProjectFile(id);
  auto projectData = juce::JSON::parse(projectFile);

  auto graphData = juce::JSON::parse(graphDataJson);
  projectData.getDynamicObject()->setProperty("graph", graphData);

  auto projectDataString = juce::JSON::toString(projectData);
  projectFile.replaceWithData(projectDataString.getCharPointer(),
                              projectDataString.getNumBytesAsUTF8());
}

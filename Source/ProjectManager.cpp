#include "ProjectManager.h"

// TODO replace projectId/index.json by projectId.json

// TODO consider replacing this manual files manipulation by some proper
// database like SQLite

ProjectManager::ProjectManager() { projectsDirectoryPath = "Modulee/projects"; }

juce::String ProjectManager::getProjects() {
  auto folder =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectsDirectoryPath);
  folder.createDirectory();

  juce::Array<juce::File> directories;
  folder.findChildFiles(directories, juce::File::findDirectories, false);

  juce::var jsonArray = juce::var(juce::Array<juce::var>{});

  for (const auto &directory : directories) {
    auto file = juce::File(directory).getChildFile("index.json");
    auto fileJson = juce::JSON::parse(file);
    juce::DynamicObject::Ptr projectObject = new juce::DynamicObject();
    projectObject->setProperty("id",
                               fileJson.getProperty("id", false).toString());
    projectObject->setProperty("name",
                               fileJson.getProperty("name", false).toString());
    jsonArray.append(juce::var(projectObject.get()));
  }

  return juce::JSON::toString(jsonArray);
}

void ProjectManager::deleteProject(juce::String id) {
  auto filePath = projectsDirectoryPath + "/" + id;
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(filePath);
  file.deleteRecursively();
}

juce::String ProjectManager::getProject(juce::String id) {
  auto projectFilePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto projectFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectFilePath);
  DBG(projectFile.loadFileAsString());
  return projectFile.loadFileAsString();
}

void ProjectManager::createProject(juce::String projectDataJson) {
  auto projectData = juce::JSON::parse(projectDataJson);
  auto id = projectData.getProperty("id", false).toString();

  auto filePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(filePath);

  file.createDirectory();

  // These parse and toString probably aren't needed. It's here just to ensure
  // consistent encoding during development.
  auto data = juce::JSON::toString(projectData);
  file.replaceWithData(data.getCharPointer(), data.getNumBytesAsUTF8());
}

void ProjectManager::renameProject(juce::String id, juce::String name) {
  auto projectFilePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto projectFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectFilePath);

  auto projectData = juce::JSON::parse(projectFile);
  projectData.getDynamicObject()->setProperty("name", name);

  auto projectDataString = juce::JSON::toString(projectData);
  projectFile.replaceWithData(projectDataString.getCharPointer(),
                              projectDataString.getNumBytesAsUTF8());
}

void ProjectManager::updateProjectGraphData(juce::String id,
                                            juce::String graphDataJson) {
  auto projectFilePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto projectFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectFilePath);

  auto graphData = juce::JSON::parse(graphDataJson);
  auto projectData = juce::JSON::parse(projectFile);
  projectData.getDynamicObject()->setProperty("graphData", graphData);

  auto projectDataString = juce::JSON::toString(projectData);
  projectFile.replaceWithData(projectDataString.getCharPointer(),
                              projectDataString.getNumBytesAsUTF8());
}

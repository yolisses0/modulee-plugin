#include "ProjectManager.h"

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
    projectObject->setProperty("id", fileJson.getProperty("id", "").toString());
    projectObject->setProperty("name",
                               fileJson.getProperty("name", "").toString());
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
  auto filePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(filePath);
  // That parse and toString probably aren't needed. It's here
  // just to ensure consisted encoding during development.
  auto fileJson = juce::JSON::parse(file);
  return juce::JSON::toString(fileJson);
}

void ProjectManager::createProject(juce::String projectDataJson) {
  auto projectData = juce::JSON::parse(projectDataJson);
  auto id = projectData.getProperty("id", false).toString();

  auto file =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectsDirectoryPath + "/" + id + "/index.json");

  file.createDirectory();
  // That parse and toString probably aren't needed. It's here
  // just to ensure consisted encoding during development.

  auto data = juce::JSON::toString(projectData);
  file.replaceWithData(data.getCharPointer(), data.getNumBytesAsUTF8());
}
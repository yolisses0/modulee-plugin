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

juce::var ProjectManager::getProjectCommands(juce::String projectId) {
  auto commandsDirectoryPath =
      projectsDirectoryPath + "/" + projectId + "/commands";
  auto commandsDirectory =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(commandsDirectoryPath);

  juce::Array<juce::File> commandFiles;
  commandsDirectory.findChildFiles(commandFiles, juce::File::findFiles, false);

  juce::var commandsData = juce::var(juce::Array<juce::var>{});
  for (const auto &commandFile : commandFiles) {
    auto commandData = juce::JSON::parse(commandFile);
    commandsData.append(commandData);
  }

  return commandsData;
}

juce::String ProjectManager::getProject(juce::String id) {
  auto projectFilePath = projectsDirectoryPath + "/" + id + "/index.json";
  auto projectFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectFilePath);
  auto projectData = juce::JSON::parse(projectFile);

  auto dynamicObject = projectData.getDynamicObject();
  auto commandsData = getProjectCommands(id);
  dynamicObject->setProperty("commands", commandsData);

  return juce::JSON::toString(projectData);
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

void ProjectManager::addCommand(juce::String commandDataJson) {
  auto commandData = juce::JSON::parse(commandDataJson);
  auto id = commandData.getProperty("id", false).toString();
  auto projectId = commandData.getProperty("projectId", false).toString();

  auto filePath =
      projectsDirectoryPath + "/" + projectId + "/commands/" + id + ".json";
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(filePath);
  file.createDirectory();

  // These parse and toString probably aren't needed. It's here just to ensure
  // consistent encoding during development.
  auto data = juce::JSON::toString(commandData);
  file.replaceWithData(data.getCharPointer(), data.getNumBytesAsUTF8());
}
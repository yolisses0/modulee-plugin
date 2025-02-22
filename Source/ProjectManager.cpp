#include "ProjectManager.h"

ProjectManager::ProjectManager() { projectsDirectoryPath = "Modulee/projects"; }

juce::File ProjectManager::getSavedDataFile() {
  auto savedDataFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile("Modulee/savedData.json");

  savedDataFile.createDirectory();

  return savedDataFile;
}

juce::String ProjectManager::getProjects() {
  auto folder =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile(projectsDirectoryPath);
  folder.createDirectory();

  juce::Array<juce::File> files;
  folder.findChildFiles(files, juce::File::findFiles, false);

  juce::var jsonArray = juce::var(juce::Array<juce::var>{});

  for (const auto &file : files) {

    auto fileJson = juce::JSON::parse(file);

    juce::DynamicObject::Ptr projectObject = new juce::DynamicObject();
    projectObject->setProperty("id", fileJson.getProperty("id", "").toString());
    projectObject->setProperty("name",
                               fileJson.getProperty("name", "").toString());
    jsonArray.append(juce::var(projectObject.get()));
  }

  return juce::JSON::toString(jsonArray);
}

juce::String ProjectManager::getProject(juce::String id) {
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(projectsDirectoryPath + "/" + id + ".json");
  // That parse and toString probably aren't needed. It's here
  // just to ensure consisted encoding during development.
  auto fileJson = juce::JSON::parse(file);
  return juce::JSON::toString(fileJson);
}

void ProjectManager::createProject(juce::String projectDataJson) {
  auto projectData = juce::JSON::parse(projectDataJson);
  auto id = projectData.getProperty("id", false).toString();

  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile(projectsDirectoryPath + "/" + id + ".json");
  // That parse and toString probably aren't needed. It's here
  // just to ensure consisted encoding during development.

  auto data = juce::JSON::toString(projectData);
  file.replaceWithData(data.getCharPointer(), data.getNumBytesAsUTF8());
}

void ProjectManager::setSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {

  juce::String dataToSave = args[0].toString();
  juce::File savedDataFile = getSavedDataFile();
  if (savedDataFile.replaceWithText(dataToSave)) {
    completion(true);
  } else {
    completion(false);
  }
}

void ProjectManager::getSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {
  auto savedDataFile = getSavedDataFile();
  juce::String loadedData = savedDataFile.loadFileAsString();
  completion(loadedData);
}
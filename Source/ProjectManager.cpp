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
  for (const auto &file : files) {
    DBG("File: " + file.getFileName());
  }

  return juce::String();
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
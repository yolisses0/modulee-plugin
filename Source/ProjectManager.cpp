#include "ProjectManager.h"

juce::File ProjectManager::getSavedDataFile() {
  auto savedDataFile =
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
          .getChildFile("Modulee/savedData.json");

  auto isFolderCreated = savedDataFile.createDirectory();

  if (isFolderCreated.failed()) {
    DBG("Failed to create the parent folder");
  }

  return savedDataFile;
}

void ProjectManager::setSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {
  DBG("setSavedData start");

  // Check if the input argument is valid
  if (args.size() > 0 && args[0].isString()) {
    juce::String dataToSave = args[0].toString();

    juce::File savedDataFile = getSavedDataFile();

    // Save the string to the file
    if (savedDataFile.replaceWithText(dataToSave)) {
      DBG("Data saved to file: " << savedDataFile.getFullPathName());
      completion(true); // Indicate success to the JavaScript caller
    } else {
      DBG("Failed to save data to file!");
      completion(false); // Indicate failure to the JavaScript caller
    }
  } else {
    DBG("Invalid argument: Expected a string!");
    completion(false); // Indicate failure to the JavaScript caller
  }

  DBG("setSavedData end");
}

void ProjectManager::getSavedData(
    const juce::Array<juce::var> &args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion) {
  DBG("getSavedData start");

  auto savedDataFile = getSavedDataFile();
  // Use the constant file path
  if (savedDataFile.existsAsFile()) {
    // Load the string from the file
    juce::String loadedData = savedDataFile.loadFileAsString();
    DBG("Data loaded from file: " << loadedData);
    completion(
        loadedData); // Pass the loaded data back to the JavaScript caller
  } else {
    DBG("File does not exist!");
    completion(juce::var()); // Return an empty value to indicate failure
  }

  DBG("getSavedData end");
}
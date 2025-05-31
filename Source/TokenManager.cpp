#include "TokenManager.h"

TokenManager::TokenManager() {
  tokenFile =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Modulee/token");
  tokenFile.getParentDirectory().createDirectory();
}

void TokenManager::saveToken(const juce::String &token) {
  juce::ScopedLock lock(tokenMutex);
  tokenFile.replaceWithText(token);
}

std::optional<juce::String> TokenManager::getToken() const {
  juce::ScopedLock lock(tokenMutex);
  if (tokenFile.existsAsFile()) {
    auto token = tokenFile.loadFileAsString().trim();
    if (!token.isEmpty())
      return token;
  }
  return std::nullopt;
}

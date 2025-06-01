#include "AuthTokenManager.h"

AuthTokenManager::AuthTokenManager() {
  file =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Modulee/authToken");
  file.getParentDirectory().createDirectory();
}

void AuthTokenManager::setAuthToken(const juce::String authToken) {
  juce::ScopedLock lock(mutex);
  file.replaceWithText(authToken);
}

std::optional<juce::String> AuthTokenManager::getAuthToken() const {
  juce::ScopedLock lock(mutex);
  if (file.existsAsFile()) {
    auto token = file.loadFileAsString().trim();
    if (!token.isEmpty())
      return token;
  }
  return std::nullopt;
}

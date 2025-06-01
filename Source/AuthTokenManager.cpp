#include "AuthTokenManager.h"

AuthTokenManager::AuthTokenManager() {
  tokenFile =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Modulee/token");
  tokenFile.getParentDirectory().createDirectory();
}

void AuthTokenManager::setAuthToken(const juce::String &token) {
  juce::ScopedLock lock(tokenMutex);
  tokenFile.replaceWithText(token);
}

std::optional<juce::String> AuthTokenManager::getAuthToken() const {
  juce::ScopedLock lock(tokenMutex);
  if (tokenFile.existsAsFile()) {
    auto token = tokenFile.loadFileAsString().trim();
    if (!token.isEmpty())
      return token;
  }
  return std::nullopt;
}

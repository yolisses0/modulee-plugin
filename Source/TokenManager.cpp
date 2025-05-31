#include "TokenManager.h"

class TokenManager {
public:
  TokenManager() {
    tokenFile =
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Modulee/token");
    tokenFile.getParentDirectory().createDirectory();
  }

  void saveToken(const juce::String &token) {
    juce::ScopedLock lock(tokenMutex);
    tokenFile.replaceWithText(token);
  }

  std::optional<juce::String> getToken() const {
    juce::ScopedLock lock(tokenMutex);
    if (tokenFile.existsAsFile()) {
      auto token = tokenFile.loadFileAsString().trim();
      if (!token.isEmpty())
        return token;
    }
    return std::nullopt;
  }

private:
  juce::File tokenFile;
  juce::CriticalSection tokenMutex;
};

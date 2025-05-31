#pragma once
#include <JuceHeader.h>

class TokenManager {
public:
  TokenManager();

  void saveToken(const juce::String &token);

  std::optional<juce::String> getToken() const;

private:
  juce::File tokenFile;
  juce::CriticalSection tokenMutex;
};

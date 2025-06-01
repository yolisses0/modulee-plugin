#pragma once
#include <JuceHeader.h>

class AuthTokenManager {
public:
  AuthTokenManager();
  void setAuthToken(const juce::String &authToken);
  std::optional<juce::String> getAuthToken() const;

private:
  juce::File file;
  juce::CriticalSection mutex;
};

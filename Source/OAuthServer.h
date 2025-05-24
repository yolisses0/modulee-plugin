// OAuthServer.h
#pragma once
#include "../Vendor/httplib.h"
#include <juce_core/juce_core.h>

class OAuthServer : public juce::Thread {
public:
  OAuthServer();
  void run() override;
  void stop();
};
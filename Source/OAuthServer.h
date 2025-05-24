// OAuthServer.h
#pragma once
#include "../Vendor/httplib.h"
#include <juce_core/juce_core.h>

class OAuthServer : public juce::Thread {
public:
  OAuthServer();
  ~OAuthServer() override;

  void run() override;
  void stop();

private:
  std::unique_ptr<httplib::Server> server;
};
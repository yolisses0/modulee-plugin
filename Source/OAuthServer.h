#pragma once
#include "../Vendor/httplib.h"
#include <functional>
#include <juce_core/juce_core.h>

class OAuthServer : public juce::Thread {
public:
  OAuthServer();
  ~OAuthServer() override;

  void run() override;
  void stop();

  std::function<void(const std::string &)> onCodeReceived;

private:
  std::unique_ptr<httplib::Server> server;
};
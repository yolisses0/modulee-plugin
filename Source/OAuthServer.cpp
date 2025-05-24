#include "OAuthServer.h"

OAuthServer::OAuthServer() : juce::Thread("OAuthServer") {}

OAuthServer::~OAuthServer() { stop(); }

void OAuthServer::run() {
  server = std::make_unique<httplib::Server>();

  server->Get(
      "/signIn", [this](const httplib::Request &req, httplib::Response &res) {
        res.set_content("Successfully signed in. You can close this tab now.",
                        "text/plain");

        std::string code = req.get_param_value("code");
        if (onCodeReceived) {
          onCodeReceived(code);
        }
      });

  server->Get("/ping", [](const httplib::Request &, httplib::Response &res) {
    res.set_content("pong", "text/plain");
  });

  juce::Logger::writeToLog("Starting server on localhost:39057");
  if (!server->listen("127.0.0.1", 39057)) {
    juce::Logger::writeToLog("Failed to start server on localhost:39057");
  }
}

void OAuthServer::stop() {
  if (server) {
    server->stop();
    server.reset();
    juce::Logger::writeToLog("OAuthServer stopped.");
  }
  signalThreadShouldExit();
  waitForThreadToExit(5000); // Wait up to 5 seconds
}
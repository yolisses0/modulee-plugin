#include "OAuthServer.h"

OAuthServer::OAuthServer() : juce::Thread("OAuthServer") {}

void OAuthServer::run() {
  httplib::Server server;

  // Handle GET request to any path
  server.Get("/.*", [](const httplib::Request &, httplib::Response &res) {
    res.set_content("pong", "text/plain");
  });

  // Start server on localhost:8080
  if (!server.listen("127.0.0.1", 8080)) {
    juce::Logger::writeToLog("Failed to start server on localhost:8080");
  }
}

void OAuthServer::stop() {
  signalThreadShouldExit();
  // httplib::Server handles shutdown when the thread exits
}
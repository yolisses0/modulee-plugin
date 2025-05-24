#include "OAuthServer.h"

OAuthServer::OAuthServer() : juce::Thread("OAuthServer") {}

OAuthServer::~OAuthServer() { stop(); }

void OAuthServer::run() {
  server = std::make_unique<httplib::Server>();

  // Handle GET request to any path
  server->Get("/.*", [](const httplib::Request &, httplib::Response &res) {
    res.set_content("pong", "text/plain");
  });

  // Start server on localhost:39057
  juce::Logger::writeToLog("Starting server on localhost:39057");
  if (!server->listen("127.0.0.1", 39057)) {
    juce::Logger::writeToLog("Failed to start server on localhost:39057");
  }
}

void OAuthServer::stop() {
  if (server) {
    // Explicitly stop the server
    server->stop();
    server.reset();
    juce::Logger::writeToLog("OAuthServer stopped.");
  }
  // Signal the thread to exit
  signalThreadShouldExit();
  // Wait for the thread to exit (with a timeout to avoid hanging)
  waitForThreadToExit(5000); // Wait up to 5 seconds
}
#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <cstdint>
#include <string>

// Forward declarations
class AsyncWebServer;
class AsyncWebServerRequest;
class SystemManager;

class WebServerManager {
public:
    WebServerManager(uint16_t port = 80);
    ~WebServerManager();

    void initialize(SystemManager* systemManager);
    void start();
    void stop();
    AsyncWebServer& getServer();

    bool authenticateRequest(AsyncWebServerRequest* request);

private:
    void setupStaticFiles();
    void setupRESTEndpoints();
    void setupConfigurationEndpoints();
    void setupErrorHandling();

    AsyncWebServer* server;
    SystemManager* systemManager;
    bool isServerRunning;
};

#endif // WEBSERVERMANAGER_H

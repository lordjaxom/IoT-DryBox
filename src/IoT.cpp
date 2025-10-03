#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include "IoT.hpp"
#include "Logger.hpp"
#include "String.hpp"

static String toHostname(String clientId)
{
    clientId.toLowerCase();
    return str("iot-", clientId);
}

IoTClass::IoTClass(char const* clientId, char const* wiFiSsid, char const* wiFiPassword) noexcept
        : clientId_(clientId),
          hostname_(toHostname(clientId)),
          watchdogTimer_([] { ESP.restart(); }),
          wiFiSsid_(wiFiSsid),
          wiFiPassword_(wiFiPassword),
          wiFiReconnectTimer_([this] { connectToWiFi(); })
{
}

void IoTClass::begin()
{
    Logger.begin();

    log("starting ESP-IoT based application");

    // No watchdog since WiFi disconnect shouldn't stop DryBox
    // watchdogTimer_.start(watchdogDelay);

    WiFi.onEvent([](WiFiEvent_t const event) {
        switch (event) {
            case WIFI_EVENT_STAMODE_GOT_IP:
                IoT.wiFiConnected();
                break;
            case WIFI_EVENT_STAMODE_DISCONNECTED:
                IoT.wiFiDisconnected();
                break;
            default:
                break;
        }
    });

    connectToWiFi();

    ArduinoOTA.onStart([this]() { endEvent(); });

    beginEvent();
    beginEvent.clear();
}

void IoTClass::loop()
{
    ArduinoOTA.handle();

    loopEvent();
}

void IoTClass::connectToWiFi()
{
    log("connecting to WiFi at ", wiFiSsid_, " as ", hostname_);

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.hostname(hostname_);
    WiFi.begin(wiFiSsid_, wiFiPassword_);
}

void IoTClass::wiFiConnected()
{
    log("connection to WiFi established as ", WiFi.localIP());

    wiFiReconnectTimer_.stop();
    watchdogTimer_.stop();

    ArduinoOTA.begin();

    connectEvent();
}

void IoTClass::wiFiDisconnected()
{
    log("connection to WiFi lost");

    wiFiReconnectTimer_.start(reconnectDelay);
    watchdogTimer_.start(watchdogDelay);

    disconnectEvent();
}

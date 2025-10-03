#ifndef ESP8266_IOT_IOT_HPP
#define ESP8266_IOT_IOT_HPP

#include <stdint.h>

#include "Event.hpp"
#include "Timer.hpp"
#include "String.hpp"

class IoTClass
{
    static constexpr uint32_t watchdogDelay = 60000;
    static constexpr uint32_t reconnectDelay = 5000;

public:
    IoTClass(char const* clientId, char const* wiFiSsid, char const* wiFiPassword) noexcept;
    IoTClass(IoTClass const&) = delete;

    String const& clientId() const { return clientId_; }

    void begin();
    void loop();

    OneShotEvent<void()> beginEvent;
    OneShotEvent<void()> endEvent;
    Event<void()> connectEvent;
    Event<void()> disconnectEvent;
    Event<void()> loopEvent;

private:
    void connectToWiFi();

    void wiFiConnected();
    void wiFiDisconnected();

    String const clientId_; // must stay constant
    String const hostname_; // must stay constant
    Timer watchdogTimer_;
    char const* wiFiSsid_;
    char const* wiFiPassword_;
    Timer wiFiReconnectTimer_;
};

extern IoTClass IoT;

#endif // ESP8266_IOT_IOT_HPP

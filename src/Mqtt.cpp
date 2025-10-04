#include <ESP8266WiFi.h>

#include "IoT.hpp"
#include "Logger.hpp"
#include "Mqtt.hpp"

static String toTopic(String clientId)
{
    clientId.replace('-', '/');
    return clientId;
}

Mqtt::Mqtt(char const* mqttIp, uint16_t mqttPort) noexcept
        : mqttIp_(mqttIp),
          mqttPort_(mqttPort),
          topic_(toTopic(IoT.clientId())),
          willTopic_(str("tele/", topic_, "/LWT")),
          reconnectTimer_([this] { connectToMqtt(); }),
          connected_(IoT.connectEvent.subscribe([this] { connectToMqtt(); })),
          disconnected_(IoT.disconnectEvent.subscribe([this] { wiFiDisconnected(); }))
{
    client_.onConnect([this](bool) { mqttConnected(); });
    client_.onDisconnect([this](AsyncMqttClientDisconnectReason) { mqttDisconnected(); });
    client_.onMessage(
            [this](char const* topic, char const* payload, AsyncMqttClientMessageProperties, size_t length, size_t,
                   size_t) {
                mqttMessage(topic, payload, length);
            });
    client_.setClientId(IoT.clientId().c_str());
    client_.setServer(mqttIp_, mqttPort_);
    client_.setWill(willTopic_.c_str(), 1, true, "Offline");
}

void Mqtt::publish(String const& topic, String const& payload, bool retain)
{
    publish(topic, payload.c_str(), retain);
}

void Mqtt::publish(const String& topic, const char* payload, bool retain)
{
    if (!client_.connected()) {
        return;
    }

    log("publishing ", payload, " to ", topic);

    client_.publish(topic.c_str(), 1, retain, payload);
}

void Mqtt::subscribe(String topic, Subscriber handler)
{
    auto it = subscriptions_.emplace(std::move(topic), std::move(handler));
    if (client_.connected() && subscriptions_.count(it->first) == 1) {
        mqttSubscribe(it->first);
    }
}

void Mqtt::subscribeCommand(const char* command, Subscriber handler)
{
    subscribe(str("cmnd/", topic_, "/", command), std::move(handler));
}

void Mqtt::connectToMqtt()
{
    log("connecting to MQTT broker at ", mqttIp_, " as ", IoT.clientId());

    client_.connect();
}

void Mqtt::wiFiDisconnected()
{
    reconnectTimer_.stop();
}

void Mqtt::mqttConnected()
{
    log("connection to MQTT broker established");

    reconnectTimer_.stop();
    publish(willTopic_, "Online", true);

    for (auto it = subscriptions_.cbegin(); it != subscriptions_.cend(); it = subscriptions_.upper_bound(it->first)) {
        mqttSubscribe(it->first);
    }

    connectEvent();
}

void Mqtt::mqttDisconnected()
{
    log("connection to MQTT broker lost");

    if (WiFi.isConnected()) {
        reconnectTimer_.start(reconnectDelay);
    }

    disconnectEvent();
}

void Mqtt::mqttMessage(char const* topic, char const* payload, size_t const length)
{
    String message;
    message.concat(payload, length);

    log("received ", message, " from ", topic);

    auto [begin, end] = subscriptions_.equal_range(topic);
    std::for_each(begin, end, [&](auto const& pair) { pair.second(message); });
}

void Mqtt::mqttSubscribe(const String& topic)
{
    log("subscribing to ", topic);

    client_.subscribe(topic.c_str(), 0);
}
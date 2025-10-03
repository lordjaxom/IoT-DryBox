#ifndef ESP8266_IOT_MQTT_HPP
#define ESP8266_IOT_MQTT_HPP

#include <stdint.h>
#include <map>

#include <AsyncMqttClient.h>

#include "Event.hpp"
#include "String.hpp"
#include "Timer.hpp"

class Mqtt {
    static constexpr uint32_t reconnectDelay = 5000;

    using Subscriber = std::function<void(String payload)>;

public:
    explicit Mqtt(char const* mqttIp, uint16_t mqttPort = 1883) noexcept;
    Mqtt(Mqtt const&) = delete;

    [[nodiscard]] String const& topic() const { return topic_; }

    void publish(String const& topic, String const& payload, bool retain = false);
    void publish(String const& topic, char const* payload, bool retain = false);
    void subscribe(String topic, Subscriber handler);

    template<typename T>
    void publishState(char const* state, T&& payload)
    {
        publish(str("stat/", topic_, "/", state), std::forward<T>(payload));
    }

    template<typename T>
    void publishTelemetry(char const* state, T&& payload)
    {
        publish(str("tele/", topic_, "/", state), std::forward<T>(payload));
    }

    void subscribeCommand(char const* command, Subscriber handler);

    Event<void()> connectEvent;
    Event<void()> disconnectEvent;

private:
    void connectToMqtt();

    void wiFiDisconnected();
    void mqttConnected();
    void mqttDisconnected();
    void mqttMessage(char const* topic, char const* payload, size_t length);
    void mqttSubscribe(String const& topic);

    char const* mqttIp_;
    uint16_t mqttPort_;
    String const topic_; // must stay constant
    String const willTopic_; // must stay constant
    Timer reconnectTimer_;
    AsyncMqttClient client_;
    std::multimap<String, Subscriber> subscriptions_;
    Subscription connected_;
    Subscription disconnected_;
};

#endif // ESP8266_IOT_MQTT_HPP

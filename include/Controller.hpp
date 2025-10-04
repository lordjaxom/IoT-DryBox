#ifndef ESP8266_IOT_CONTROLLER_HPP
#define ESP8266_IOT_CONTROLLER_HPP

#include <stdint.h>

#include "Event.hpp"
#include "Timer.hpp"

class Controller;
class DHT20;
class Display;
class DS18B20;
class Mqtt;
class Output;
class PushButton;

class ControllerMode
{
public:
    explicit ControllerMode(Controller& c) noexcept;
    virtual ~ControllerMode() = default;

    [[nodiscard]] virtual float getTargetTemperature() const = 0;

    virtual void update() = 0;
    virtual void next() = 0;
    virtual void upClicked() = 0;
    virtual void downClicked() = 0;

protected:
    Controller& c;
};

class ControllerModeOff final : public ControllerMode
{
public:
    explicit ControllerModeOff(Controller& c) noexcept;

    [[nodiscard]] float getTargetTemperature() const override { return 0; }

    void update() override;
    void next() override;
    void upClicked() override {}
    void downClicked() override {}
};

class ControllerModeOn final : public ControllerMode
{
public:
    explicit ControllerModeOn(Controller& c) noexcept;

    [[nodiscard]] float getTargetTemperature() const override { return 45.0f; }

    void update() override;
    void next() override;
    void upClicked() override;
    void downClicked() override;
};

class Controller
{
    static constexpr uint32_t updateDelay = 1000;
    static constexpr uint32_t telemetryDelay = 5000;

    friend class ControllerModeOff;
    friend class ControllerModeOn;

public:
    Controller(
        Mqtt& mqtt,
        PushButton& onOffButton,
        PushButton& upButton,
        PushButton& downButton,
        DHT20& dht20,
        DS18B20& ds18b20,
        Display& display
    );
    Controller(Controller const&) = delete;

private:
    void update() const;
    void telemetry() const;
    void onOffClicked(unsigned clicks) const;
    void upClicked(unsigned clicks) const;
    void downClicked(unsigned clicks) const;

    Mqtt& mqtt_;
    DHT20& dht20_;
    DS18B20& ds18b20_;
    Display& display_;
    Timer updateTimer_;
    Timer telemetryTimer_;
    Subscription onOffClicked_;
    Subscription upClicked_;
    Subscription downClicked_;
    std::unique_ptr<ControllerMode> mode_{};
};

#endif

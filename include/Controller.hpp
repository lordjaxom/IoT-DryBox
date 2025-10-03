#ifndef ESP8266_IOT_CONTROLLER_HPP
#define ESP8266_IOT_CONTROLLER_HPP

#include <stdint.h>

#include "Event.hpp"
#include "Timer.hpp"

class Controller;
class Display;
class Mqtt;
class PushButton;

class ControllerMode
{
public:
    explicit ControllerMode(Controller& c) noexcept;
    virtual ~ControllerMode() = default;

    virtual void update() = 0;
    virtual void next() = 0;

protected:
    Controller& c;
};

class ControllerModeOff final : public ControllerMode
{
public:
    explicit ControllerModeOff(Controller& c) noexcept;
    void update() override;
    void next() override;
};

class ControllerModeOn final : public ControllerMode
{
public:
    explicit ControllerModeOn(Controller& c) noexcept;
    void update() override;
    void next() override;
};

class Controller
{
    static constexpr uint32_t updateDelay = 100;

    friend class ControllerModeOff;
    friend class ControllerModeOn;

public:
    Controller(Mqtt& mqtt, PushButton& onOffButton, Display& display);
    Controller(Controller const&) = delete;

private:
    void update();
    void onOffClicked(unsigned clicks);

    Mqtt& mqtt_;
    Display& display_;
    Timer updateTimer_;
    Subscription onOffClicked_;
    std::unique_ptr<ControllerMode> mode_{};
};

#endif

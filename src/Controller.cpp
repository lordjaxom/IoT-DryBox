#include <memory>

#include "IoT.hpp"
#include "Controller.hpp"
#include "Display.hpp"
#include "DHT20.hpp"
#include "DS18B20.hpp"
#include "Json.hpp"
#include "Mqtt.hpp"
#include "Output.hpp"
#include "PushButton.hpp"

#include "Heating.h"

ControllerMode::ControllerMode(Controller& c) noexcept
    : c{c}
{
}

ControllerModeOff::ControllerModeOff(Controller& c) noexcept
    : ControllerMode{c}
{
    c.mqtt_.publishState("POWER", "OFF");
}

void ControllerModeOff::update()
{
    c.display_.showStandbyPanel(c.ds18b20_.getTemperature(), c.dht20_.getHumidity());
}

void ControllerModeOff::onOffClicked()
{
    c.mode_ = std::make_unique<ControllerModeOn>(c);
}

ControllerModeOn::ControllerModeOn(Controller& c) noexcept
    : ControllerMode{c}
{
    c.mqtt_.publishState("POWER", "ON");
}

float ControllerModeOn::getSetpoint() const
{
    return c.setpoint_;
}

void ControllerModeOn::update()
{
    c.display_.showHeatingPanel(c.ds18b20_.getTemperature(), c.dht20_.getHumidity());
}

void ControllerModeOn::onOffClicked()
{
    c.mode_ = std::make_unique<ControllerModeOff>(c);
}

void ControllerModeOn::upClicked()
{
    c.mode_ = std::make_unique<ControllerModeSet>(c, 1.0f);
}

void ControllerModeOn::downClicked()
{
    c.mode_ = std::make_unique<ControllerModeSet>(c, -1.0f);
}

ControllerModeSet::ControllerModeSet(Controller& c, float const offset) noexcept
    : ControllerMode{c},
      expireTimer_{[&] { c.mode_ = std::make_unique<ControllerModeOn>(c); }}
{
    adjust(offset);
}

float ControllerModeSet::getSetpoint() const
{
    return c.setpoint_;
}

void ControllerModeSet::update()
{
}

void ControllerModeSet::onOffClicked()
{
    c.mode_ = std::make_unique<ControllerModeOff>(c);
}

void ControllerModeSet::adjust(float const offset)
{
    c.setpoint_ += offset;
    if (c.setpoint_ < 40.0f) c.setpoint_ = 40.0f;
    if (c.setpoint_ > 70.0f) c.setpoint_ = 70.0f;

    c.display_.showSetpointPanel(c.setpoint_);

    expireTimer_.start(expireDelay);
}

Controller::Controller(
    Mqtt& mqtt,
    PushButton& onOffButton,
    PushButton& upButton,
    PushButton& downButton,
    AnalogOutput& heater,
    DHT20& dht20,
    DS18B20& ds18b20,
    Display& display
) : mqtt_{mqtt},
    heater_{heater},
    dht20_{dht20},
    ds18b20_{ds18b20},
    display_{display},
    updateTimer_{updateDelay, true, [this] { update(); }},
    telemetryTimer_{telemetryDelay, true, [this] { telemetry(); }},
    onOffClicked_{onOffButton.clickedEvent.subscribe([this](auto const clicks) { onOffClicked(clicks); })},
    upClicked_{upButton.clickedEvent.subscribe([this](auto const clicks) { upClicked(clicks); })},
    downClicked_{downButton.clickedEvent.subscribe([this](auto const clicks) { downClicked(clicks); })}
{
    IoT.beginEvent += [this]() { begin(); };
}

void Controller::begin()
{
    mqtt_.subscribeCommand("POWER", [this](auto const& payload) { powerReceived(payload); });
    mode_ = std::make_unique<ControllerModeOff>(*this);
}

void Controller::update() const
{
    mode_->update();
    controlHeater(mode_->getSetpoint());
}

void Controller::telemetry() const
{
    JsonDocument doc;
    doc["POWER"] = mode_->isPowered() ? "ON" : "OFF";
    doc["PWM"] = heater_.get();
    mqtt_.publishTelemetry("STATE", str(doc));

    doc.clear();
    doc["DHT20"]["Temperature"] = dht20_.getTemperature();
    doc["DHT20"]["Humidity"] = dht20_.getHumidity();
    doc["DS18B20"]["Temperature"] = ds18b20_.getTemperature();
    mqtt_.publishTelemetry("SENSOR", str(doc));
}

void Controller::onOffClicked(unsigned const clicks) const
{
    if (clicks == 1) mode_->onOffClicked();
}

void Controller::upClicked(unsigned const clicks) const
{
    if (clicks == 1) mode_->upClicked();
}

void Controller::downClicked(unsigned const clicks) const
{
    if (clicks == 1) mode_->downClicked();
}

void Controller::powerReceived(String const& payload)
{
    if (payload == "ON" && !mode_->isPowered()) mode_ = std::make_unique<ControllerModeOn>(*this);
    else if (payload == "OFF" && mode_->isPowered()) mode_ = std::make_unique<ControllerModeOff>(*this);
    else mqtt_.publishState("POWER", mode_->isPowered() ? "ON" : "OFF");
}

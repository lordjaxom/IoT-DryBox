#include <memory>

#include "IoT.hpp"
#include "Controller.hpp"

#include <Output.hpp>

#include "Display.hpp"
#include "DHT20.hpp"
#include "DS18B20.hpp"
#include "Logger.hpp"
#include "Mqtt.hpp"
#include "PushButton.hpp"

ControllerMode::ControllerMode(Controller& c) noexcept
    : c{c}
{
}

ControllerModeOff::ControllerModeOff(Controller& c) noexcept
    : ControllerMode{c}
{
    c.heater_.set(0);
    c.mqtt_.publishState("POWER", "OFF");
}

void ControllerModeOff::update()
{
    c.display_.clear();
    c.display_.drawStatusOff();
    c.display_.drawTemperatureOff(c.dht20_.getTemperature());
    c.display_.drawHumidityOff(c.dht20_.getHumidity());
    c.display_.show();
}

void ControllerModeOff::next()
{
    c.mode_ = std::make_unique<ControllerModeOn>(c);
}

ControllerModeOn::ControllerModeOn(Controller& c) noexcept
    : ControllerMode{c}
{
    c.heater_.set(128);
    c.mqtt_.publishState("POWER", "ON");
}

void ControllerModeOn::update()
{
    c.display_.clear();
    c.display_.drawStatusOn();
    c.display_.drawTemperatureOn(c.dht20_.getTemperature());
    c.display_.drawHumidityOn(c.dht20_.getHumidity());
    c.display_.show();
}

void ControllerModeOn::next()
{
    c.mode_ = std::make_unique<ControllerModeOff>(c);
}

void ControllerModeOn::upClicked()
{
}

void ControllerModeOn::downClicked()
{
}

Controller::Controller(
    Mqtt& mqtt,
    PushButton& onOffButton,
    PushButton& upButton,
    PushButton& downButton,
    Output& heater,
    DHT20& dht20,
    DS18B20& ds18b20,
    Display& display
) : mqtt_{mqtt},
    heater_{heater},
    dht20_{dht20},
    ds18b20_{ds18b20},
    display_{display},
    updateTimer_{updateDelay, true, [this] { update(); }},
    onOffClicked_{onOffButton.clickedEvent.subscribe([this](auto const clicks) { onOffClicked(clicks); })},
    upClicked_{upButton.clickedEvent.subscribe([this](auto const clicks) { upClicked(clicks); })},
    downClicked_{downButton.clickedEvent.subscribe([this](auto const clicks) { downClicked(clicks); })}
{
    IoT.beginEvent += [this]() { mode_ = std::make_unique<ControllerModeOff>(*this); };
}

void Controller::update() const
{
    dht20_.update();
    ds18b20_.update();
    mode_->update();

    mqtt_.publishTelemetry("SENSOR",str(
        R"({"DHT20":{"Temperature":)", dht20_.getTemperature(), R"(,"Humidity":)", dht20_.getHumidity(), R"(},)",
        R"("DS18B20":{"Temperature":)", ds18b20_.getTemperature(), R"(}})"
    ));
}

void Controller::onOffClicked(unsigned const clicks) const
{
    mqtt_.publishState("BUTTON1", str(clicks));
    if (clicks == 1) mode_->next();
}

void Controller::upClicked(unsigned const clicks) const
{
    if (clicks == 1) mode_->upClicked();
}

void Controller::downClicked(unsigned const clicks) const
{
    if (clicks == 1) mode_->downClicked();
}

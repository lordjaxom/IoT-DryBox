#include "IoT.hpp"
#include "Controller.hpp"

#include <memory>

#include "DallasTemperature.h"
#include "DFRobot_DHT20.h"
#include "Display.hpp"
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
    log("Switching controller to OFF");
}

void ControllerModeOff::update()
{
    c.display_.clear();
    c.display_.drawStatusOff();
    c.display_.show();
}

void ControllerModeOff::next()
{
    c.mode_ = std::make_unique<ControllerModeOn>(c);
}

ControllerModeOn::ControllerModeOn(Controller& c) noexcept
    : ControllerMode{c}
{
    log("Switching controller to ON");
}

void ControllerModeOn::update()
{
    c.display_.clear();
    c.display_.drawStatusOn();
    c.display_.show();
}

void ControllerModeOn::next()
{
    c.mode_ = std::make_unique<ControllerModeOff>(c);
}

Controller::Controller(Mqtt& mqtt, PushButton& onOffButton, Display& display)
    : mqtt_{mqtt},
      display_{display},
      updateTimer_{updateDelay, true, [this] { update(); }},
      onOffClicked_{onOffButton.clickedEvent.subscribe([this](auto const clicks) { onOffClicked(clicks); })}
{
    IoT.beginEvent += [this]() { mode_ = std::make_unique<ControllerModeOff>(*this); };
}

extern DFRobot_DHT20 dht20;
extern DallasTemperature ds18b20;

void Controller::update()
{
    static int count = 0;
    if (++count % 10 == 0) {
        mqtt_.publishState("TEMP1", str(dht20.getTemperature()));
        ds18b20.requestTemperatures();
        mqtt_.publishState("TEMP2", str(ds18b20.getTempCByIndex(0)));
    }
    mode_->update();
}

void Controller::onOffClicked(unsigned const clicks)
{
    mqtt_.publishState("BUTTON1", str(clicks));

    if (clicks == 1) {
        mode_->next();
    }
}

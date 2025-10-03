#include "IoT.hpp"
#include "Controller.hpp"
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
    c.display_.drawStatusOff();
}

void ControllerModeOff::next()
{
    c.mode_.reset(new ControllerModeOn{c});
}

ControllerModeOn::ControllerModeOn(Controller& c) noexcept
    : ControllerMode{c}
{
    log("Switching controller to ON");
}

void ControllerModeOn::update()
{
    c.display_.drawStatusOn();
}

void ControllerModeOn::next()
{
    c.mode_.reset(new ControllerModeOff{c});
}

Controller::Controller(Mqtt& mqtt, PushButton& onOffButton, Display& display)
    : mqtt_{mqtt},
      display_{display},
      updateTimer_{updateDelay, true, [this] { update(); }},
      onOffClicked_{onOffButton.clickedEvent.subscribe([this](auto const clicks) { onOffClicked(clicks); })}
{
    IoT.beginEvent += [this]() { mode_.reset(new ControllerModeOff(*this)); };
}

void Controller::update()
{
    mode_->update();
}

void Controller::onOffClicked(unsigned const clicks)
{
    mqtt_.publishState("BUTTON1", str(clicks));

    if (clicks == 1) {
        mode_->next();
    }
}

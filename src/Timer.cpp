#include <utility>

#include <Arduino.h>

#include "IoT.hpp"
#include "Timer.hpp"

Timer::Timer(Handler handler) noexcept
        : Timer(0, std::move(handler))
{
}

Timer::Timer(uint32_t const timeout, Handler handler) noexcept
        : Timer(timeout, false, std::move(handler))
{
}

Timer::Timer(uint32_t const timeout, bool const repeat, Handler handler) noexcept
        : handler_(std::move(handler)),
          looped_(IoT.loopEvent.subscribe([this] { loop(); }))
{
    start(timeout, repeat);
}

void Timer::start(uint32_t const timeout, bool const repeat)
{
    timeout_ = timeout;
    repeat_ = repeat;
    startTime_ = millis();
}

void Timer::stop()
{
    timeout_ = 0;
}

void Timer::loop()
{
    if (timeout_ == 0) {
        return;
    }

    auto const now = millis();
    if (auto const elapsed = now - startTime_; elapsed >= timeout_) {
        if (repeat_) {
            startTime_ = now;
        } else {
            timeout_ = 0;
        }
        handler_();
    }
}

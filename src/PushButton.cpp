#include <utility>

#include "IoT.hpp"
#include "PushButton.hpp"

PushButton::PushButton(Handler input) noexcept
    : input_(std::move(input)),
      looped_{IoT.loopEvent.subscribe([this] { loop(); })},
      expiredTimer_{[this] { expired(); }}
{
}

void PushButton::loop() {
    if (auto const value = input_(); value_ != value) {
        value_ = value;
        if (value_) {
            expiredTimer_.start(1000);
        } else if (!finished_) {
            ++clicks_;
            clickedEvent(singleClick);
            expiredTimer_.start(200);
        } else {
            finished_ = false;
        }
    }
}

void PushButton::expired() {
    if (value_) {
        finished_ = true;
        if (clicks_ == 0) {
            clickedEvent(longClick);
        }
    } else {
        clickedEvent(clicks_);
    }
    clicks_ = 0;
}

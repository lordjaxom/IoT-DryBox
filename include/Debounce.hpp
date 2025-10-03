#ifndef ESP8266_IOT_DEBOUNCE_HPP
#define ESP8266_IOT_DEBOUNCE_HPP

#include <utility>

#include <Arduino.h>

template<typename Input>
class Debounce
{
    static constexpr uint64_t debounceDelay = 50;

    using Type = decltype(std::declval<Input>()());

public:
    explicit Debounce(Input&& input) noexcept
            : input_(std::move(input))
    {
    }

    Type operator()()
    {
        auto value = input_();
        if (value != lastValue_) {
            timestamp_ = millis();
        }
        if (millis() - timestamp_ > debounceDelay) {
            // whatever the reading is at, it's been there for longer than the debounce
            // delay, so take it as the actual current state:
            value_ = value;
        }
        lastValue_ = value;
        return value_;
    }

private:
    Input input_;
    Type value_{};
    Type lastValue_{};
    uint64_t timestamp_{};
};

template<typename Input>
Debounce<Input> debounce(Input&& input)
{
    return Debounce<Input>(std::forward<Input>(input));
}

#endif // ESP8266_IOT_DEBOUNCE_HPP

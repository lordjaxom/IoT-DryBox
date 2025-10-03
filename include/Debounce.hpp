#ifndef ESP8266_IOT_DEBOUNCE_HPP
#define ESP8266_IOT_DEBOUNCE_HPP

#include <stdint.h>
#include <utility>

template<typename Input>
class Debounce
{
    using Type = decltype(std::declval<Input>()());

public:
    explicit Debounce(Input&& input) noexcept
            : input_(std::move(input))
    {
    }

    Type operator()()
    {
        Type temp = value_ ^ input_();
        states_[0] = ~(states_[0] & temp);
        states_[1] = states_[0] ^ (states_[1] & temp);
        temp &= states_[0] & states_[1];
        value_ ^= temp;
        return value_;
    }

private:
    Input input_;
    Type value_{};
    Type states_[2]{};
};

template<typename Input>
Debounce<Input> debounce(Input&& input)
{
    return Debounce<Input>(std::forward<Input>(input));
}

#endif // ESP8266_IOT_DEBOUNCE_HPP

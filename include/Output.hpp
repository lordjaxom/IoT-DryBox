#ifndef ESP8266_IOT_OUTPUT_HPP
#define ESP8266_IOT_OUTPUT_HPP

#include <functional>

class AnalogOutput
{
    using Handler = std::function<void(uint8_t value)>;

public:
    explicit AnalogOutput(Handler handler) noexcept
        : handler_{std::move(handler)} {}

    AnalogOutput(AnalogOutput const&) = delete;

    [[nodiscard]] uint8_t get() const { return value_; }
    void set(uint8_t const value) { handler_(value_ = value); }

private:
    Handler handler_;
    uint8_t value_{};
};

#endif // ESP8266_IOT_OUTPUT_HPP
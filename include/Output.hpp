#ifndef ESP8266_IOT_OUTPUT_HPP
#define ESP8266_IOT_OUTPUT_HPP

#include <functional>

class Output
{
    using Handler = std::function<void(uint8_t value)>;

public:
    explicit Output(Handler output = nullptr) noexcept;
    Output(Output const&) = delete;

    void set(uint8_t value);

private:
    Handler output_;
};

#endif // ESP8266_IOT_OUTPUT_HPP
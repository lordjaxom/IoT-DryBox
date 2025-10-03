#ifndef ESP8266_IOT_PUSHBUTTON_HPP
#define ESP8266_IOT_PUSHBUTTON_HPP

#include <functional>

#include "Event.hpp"
#include "Timer.hpp"

class PushButton
{
    static constexpr uint32_t updateDelay = 10;

    using Handler = std::function<bool()>;

public:
    static constexpr unsigned singleClick = 0;
    static constexpr unsigned longClick = std::numeric_limits<unsigned>::max();

    explicit PushButton(Handler input) noexcept;
    PushButton(PushButton const&) = delete;

    Event<void(unsigned clicks)> clickedEvent;

private:
    void update();
    void expired();

    Handler input_;
    Timer updateTimer_;
    Timer expiredTimer_;
    bool value_{};
    unsigned clicks_{};
    bool finished_{};
};

#endif // ESP8266_IOT_PUSHBUTTON_HPP

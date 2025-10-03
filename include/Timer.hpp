#ifndef ESP8266_IOT_TIMER_HPP
#define ESP8266_IOT_TIMER_HPP

#include <functional>

#include "Event.hpp"

class Timer
{
    using Handler = std::function<void()>;

public:
    explicit Timer(Handler handler) noexcept;
    Timer(uint32_t timeout, Handler handler) noexcept;
    Timer(uint32_t timeout, bool repeat, Handler handler) noexcept;
    Timer(Timer const&) = delete;

    [[nodiscard]] bool active() const { return timeout_ > 0; }

    void start(uint32_t timeout, bool repeat = false);
    void stop();

private:
    void loop();

    std::function<void()> handler_;
    Subscription looped_;
    uint32_t timeout_{};
    bool repeat_{};
    uint32_t startTime_{};
};

#endif //ESP8266_IOT_TIMER_HPP

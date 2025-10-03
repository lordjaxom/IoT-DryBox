#ifndef ESP8266_IOT_STRING_HPP
#define ESP8266_IOT_STRING_HPP

#include <utility>

#include <WString.h>

namespace detail
{
    template<typename T>
    auto strAppend(String& result, T&& arg) -> decltype(result += std::forward<T>(arg), void())
    {
        result += std::forward<T>(arg);
    }

    inline void str(String&) {}

    template<typename Arg0, typename ...Args>
    void str(String& result, Arg0&& arg0, Args&& ...args)
    {
        strAppend(result, std::forward<Arg0>(arg0));
        str(result, std::forward<Args>(args)...);
    }

} // namespace detail

template<typename ...Args>
String str(Args&& ... args)
{
    String result;
    ::detail::str(result, std::forward<Args>(args)...);
    return result;
}

#endif //ESP8266_IOT_STRING_HPP

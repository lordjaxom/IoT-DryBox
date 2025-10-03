#include "Output.hpp"

Output::Output(Handler output) noexcept
        : output_{std::move(output)}
{
}

void Output::set(uint8_t value)
{
    if (output_) {
        output_(value);
    }
}
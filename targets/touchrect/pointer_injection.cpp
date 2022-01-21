#pragma comment(lib, "windowsapp")

#include "pointer_injection.h"

pointer_injection pointer_injection::singleton {};

pointer_injection::pointer_injection()
{
}

pointer_injection::~pointer_injection()
{
}

void pointer_injection::inject(const std::unordered_map<WORD, user_pointer>& pointers)
{
    for(auto& kvp : pointers)
    {
        const user_pointer& pointer = kvp.second;

        INPUT input[2] {
            [&pointer]
            {
                INPUT $ {};
                $.type = INPUT_MOUSE;
                [&pointer](decltype($.mi)& $)
                {
                    $ = {};
                    $.dwFlags = MOUSEEVENTF_WHEEL;
                    $.mouseData = std::get<1>(pointer.velocity);
                }($.mi);
                return std::move($);
            }(),
            [&pointer]
            {
                INPUT $ {};
                $.type = INPUT_MOUSE;
                [&pointer](decltype($.mi)& $)
                {
                    $ = {};
                    $.dwFlags = MOUSEEVENTF_HWHEEL;
                    $.mouseData = std::get<0>(pointer.velocity);
                }($.mi);
                return std::move($);
            }()
        };
        SendInput(2, input, sizeof(INPUT));
    }
}
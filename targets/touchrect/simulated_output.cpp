#pragma comment(lib, "windowsapp")

#include "simulated_output.h"
#include "imgui.h"
#include <algorithm>

simulated_output simulated_output::singleton {};

simulated_output::simulated_output()
{
}

simulated_output::~simulated_output()
{
}

void simulated_output::simulate(const std::unordered_map<WORD, user_pointer>& input)
{
    move_cursor(input);
    scroll(input);
}

void simulated_output::draw_gui()
{
    namespace ui = ImGui;
    config.mouse.draw_gui();

    ui::Separator();
    ui::Text("interval : %.3f", scroll_momentum.cached_frame_time);
    ui::Text("max_speed : %.3f", scroll_momentum.max_speed);
    ui::Text("velocity : %.3f, %.3f",
        std::get<0>(scroll_momentum.cached_pointer.velocity),
        std::get<1>(scroll_momentum.cached_pointer.velocity)
    );
    if(scroll_momentum.is_contributing())
        scroll_momentum.cached_pointer.draw_gui();
}

void simulated_output::move_cursor(const std::unordered_map<WORD, user_pointer> &input)
{
    analog_output(input, cursor_momentum, config.mouse.movement,
        [this](const user_pointer& pointer)
        {
            INPUT input[1] {
                [this, &pointer]
                {
                    INPUT $ {};
                    $.type = INPUT_MOUSE;
                    [this, &pointer](decltype($.mi)& $)
                    {
                        $ = {};
                        $.dwFlags = MOUSEEVENTF_MOVE;
                        auto dxy = config.mouse.movement.apply(pointer.velocity);
                        $.dx = std::get<0>(dxy);
                        $.dy = std::get<1>(dxy);
                    }($.mi);
                    return std::move($);
                }()
            };
            SendInput(1, input, sizeof(INPUT));
        },
        [this, &input]() { return input.size() == config.mouse.move_fingers.fingers; },
        config.mouse.enabled
    );
}

void simulated_output::scroll(const std::unordered_map<WORD, user_pointer>& input)
{
    analog_output(input, scroll_momentum, config.mouse.scroll.movement,
        [this](const user_pointer& pointer)
        {
            INPUT input[2] {
                [this, &pointer]
                {
                    INPUT $ {};
                    $.type = INPUT_MOUSE;
                    [this, &pointer](decltype($.mi)& $)
                    {
                        $ = {};
                        $.dwFlags = MOUSEEVENTF_WHEEL;
                        $.mouseData = config.mouse.scroll.movement.apply(std::get<1>(pointer.velocity));
                    }($.mi);
                    return std::move($);
                }(),
                [this, &pointer]
                {
                    INPUT $ {};
                    $.type = INPUT_MOUSE;
                    [this, &pointer](decltype($.mi)& $)
                    {
                        $ = {};
                        $.dwFlags = MOUSEEVENTF_HWHEEL;
                        $.mouseData = config.mouse.scroll.movement.apply(-std::get<0>(pointer.velocity));
                    }($.mi);
                    return std::move($);
                }()
            };
            SendInput(2, input, sizeof(INPUT));
        },
        [this, &input]() { return input.size() == config.mouse.scroll.fingers.fingers; },
        config.mouse.enabled
    );
}

void simulated_output::analog_output(
    const std::unordered_map<WORD, user_pointer>& input,
    momentum_state& momentum,
    const movement_profile& movement,
    std::function<void(const user_pointer&)> body,
    std::function<bool()> enabled,
    bool feature_enabled
) {
    if(feature_enabled && enabled())
    {
        for(auto& kvp : input)
        {
            momentum.tick(kvp.second);
            body(kvp.second);
        }
    }
    else if(feature_enabled && momentum.cached_pointer.speed() > 2)
    {
        momentum.tick(movement.momentum);
        body(momentum.cached_pointer);
    }
    else
    {
        momentum = {};
    }
}

void finger_count::draw_gui()
{
    namespace ui = ImGui;
    ui::PushID(this);
    {
        ui::DragInt("Fingers", &fingers, 1.0f, 0, 5);
        if(ui::IsItemHovered())
            ui::SetTooltip("0 turns off this feature");
    }
    ui::PopID();
}

void movement_profile::draw_gui()
{
    namespace ui = ImGui;
    ui::PushID(this);
    {
        ui::DragFloat3("V | γ | p", &speed);
        if (ui::IsItemHovered())
            ui::SetTooltip("Speed | Gamma/Acceleration | Momentum");
    }
    ui::PopID();
}

float movement_profile::apply(float in)
{
    return std::copysign(std::pow(std::abs(in * speed), gamma), in * speed);
}

t_float2 movement_profile::apply(t_float2 in)
{
    return
    {
        apply(std::get<0>(in)),
        apply(std::get<1>(in))
    };
}

void scroll_config::draw_gui()
{
    namespace ui = ImGui;
    ui::PushID(this);
    {
        ui::Text("Scrolling");
        fingers.draw_gui();
        movement.draw_gui();
    }
    ui::PopID();
}

void cursor_action::draw_gui()
{
    namespace ui = ImGui;

    ui::PushID(this);
    {
        ui::BeginChild("Action", {0,100}, true);
        {
            fingers.draw_gui();
            ui::Checkbox("Tap to click", &tap_to_click);
            ui::SameLine();
            ui::Checkbox("Touch also drags", &touch_also_drags);
            ui::DragInt("Target mouse button", &target_mouse_button);
        }
        ui::EndChild();
    }
    ui::PopID();
}

void mouse_config::draw_gui()
{
    namespace ui = ImGui;
    ui::PushID(this);
    {
        ui::Checkbox("##enabled", &enabled);
        ui::SameLine();
        if(ui::CollapsingHeader("Mouse output"))
        {
            ui::Text("Cursor movement");
            move_fingers.draw_gui();
            movement.draw_gui();

            ui::Separator();
            scroll.draw_gui();

            ui::Separator();
            if(ui::Button("+##1st"))
                actions.push_back({});

            ui::SameLine();
            ui::Text("Buttons");

            for(cursor_action& action : actions)
            {
                ui::PushID(&action);
                {
                    if(ui::Button("-"))
                    {
                        actions.erase(std::remove_if(
                            actions.begin(), actions.end(),
                            [&action](cursor_action& a)
                            {
                                return &a == &action;
                            }
                        ));
                    }
                    ui::SameLine();
                    if (ui::CollapsingHeader("Button"))
                        action.draw_gui();
                }
                ui::PopID();
            }
            if(!actions.empty())
                if(ui::Button("+##2nd"))
                    actions.push_back({});
        }
    }
    ui::PopID();
}

void momentum_state::tick(float momentum)
{
    float iv = interval_sec();

    if(record)
    {
        max_speed = cached_pointer.speed();
        max_velocity = cached_pointer.velocity;
        direction = cached_pointer.direction();
    }
    record = false;

    if(momentum <= 0 || cached_pointer.speed() < 3)
    {
        cached_pointer = {};
        return;
    }

    float reduce_speed = min(max_speed * iv, momentum * iv);

    auto reduce_by = direction * reduce_speed;
    cached_pointer.velocity = cached_pointer.velocity - reduce_by;
    cached_pointer.position = cached_pointer.position + cached_pointer.velocity;
}

void momentum_state::tick(const user_pointer& pointer)
{
    interval_sec();

    if(!record)
        cached_pointer = {};

    record = true;

    last_pointers[current_pointer] = pointer;
    current_pointer = (current_pointer + 1) % last_pointers.size();

    auto result = std::max_element(
        last_pointers.begin(), last_pointers.end(),
        [](const user_pointer& a, const user_pointer& b)
        {
            return a.speed() < b.speed();
        }
    );

    cached_pointer = *result;
}

float momentum_state::interval_sec()
{
    if(prev_timestamp == 0)
    {
        prev_timestamp = GetTickCount64();
        return 0;
    }

    auto time_stamp = GetTickCount64();
    float frame_time_ms = static_cast<float>(time_stamp - prev_timestamp);
    prev_timestamp = time_stamp;

    return cached_frame_time = frame_time_ms * 0.001;
}

bool momentum_state::is_contributing() {
    return !record && cached_pointer.speed() >= 3;
}

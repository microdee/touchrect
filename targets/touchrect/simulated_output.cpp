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

}

void simulated_output::scroll(const std::unordered_map<WORD, user_pointer>& input)
{
    auto body = [this](const user_pointer& pointer)
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
    };

    if(config.mouse.enabled && input.size() == config.mouse.scroll.fingers.fingers)
    {
        for(auto& kvp : input)
        {
            scroll_momentum.tick(kvp.second);
            body(kvp.second);
        }
    }
    else if(config.mouse.enabled && scroll_momentum.cached_pointer.get_speed() > 2)
    {
        scroll_momentum.tick(config.mouse.scroll.movement.momentum);
        body(scroll_momentum.cached_pointer);
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
        max_speed = cached_pointer.get_speed();
        max_velocity = cached_pointer.velocity;
        direction = cached_pointer.get_direction();
    }
    record = false;

    if(momentum <= 0 || cached_pointer.get_speed() < 3)
    {
        cached_pointer = {};
        return;
    }

    float reduce_speed = min(max_speed * iv, momentum * iv);

    auto reduce_by = direction * t_float2(reduce_speed, reduce_speed);
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
            return a.get_speed() < b.get_speed();
        }
    );

    cached_pointer = *result;

#if 0
    auto max_vel = prev_pointer.get_speed() > pointer.get_speed()
        ? prev_pointer.velocity
        : pointer.velocity;

    cached_pointer.velocity = max_vel;
#endif
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
    return !record && cached_pointer.get_speed() >= 3;
}

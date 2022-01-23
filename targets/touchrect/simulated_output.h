#pragma once

#include <Windows.h>

#include <unordered_map>
#include <utility>
#include <array>

#include "tuple_binary_operators.h"
#include "user_pointer.h"

struct finger_count
{
    int fingers = 0;

    void draw_gui();
};

struct movement_profile
{
    float speed = 1.0f;
    float gamma = 1.0f;
    float momentum = 0.0f;

    float apply(float in);
    void draw_gui();
};

struct scroll_config
{
    finger_count fingers {};
    movement_profile movement {1.5f, 1.15f, 400.0f};

    void draw_gui();
};

struct cursor_action
{
    finger_count fingers {};
    bool tap_to_click = false;
    bool touch_also_drags = false;
    int target_mouse_button = 0;

    void draw_gui();
};

struct mouse_config
{
    bool enabled = false;
    finger_count move_fingers {};
    movement_profile movement {};
    scroll_config scroll {};
    std::vector<cursor_action> actions {};

    void draw_gui();
};

struct momentum_state
{
    bool record = false;
    user_pointer cached_pointer = {};
    float max_speed = 0;
    t_float2 max_velocity {};
    t_float2 direction {};
    ULONGLONG prev_timestamp = 0;
    float cached_frame_time = 0;

    std::array<user_pointer, 5> last_pointers;
    int current_pointer = 0;

    float interval_sec();

    void tick(float momentum);
    void tick(const user_pointer& pointer);
    bool is_contributing();
};

struct simulated_output_config
{
    mouse_config mouse;
};

class simulated_output
{
public:
	static simulated_output singleton;

    simulated_output_config config;

	simulated_output();
	~simulated_output();
	void simulate(const std::unordered_map<WORD, user_pointer>& input);
    void draw_gui();

private:

    momentum_state scroll_momentum {};

    void scroll(const std::unordered_map<WORD, user_pointer>& input);
    void move_cursor(const std::unordered_map<WORD, user_pointer>& input);
};
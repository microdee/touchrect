
#pragma once

#include <Windows.h>
#include <variant>
#include <optional>
#include <memory>
#include "common_concepts.h"
#include <functional>
#include "tuple_binary_operators.h"

struct user_pointer_source
{
	HWND hWnd;
	UINT msg;
	WPARAM wParam;
	LPARAM lParam;
};

class user_pointer
{
public:
	user_pointer() {}
	user_pointer(const user_pointer_source& in_source);
	user_pointer(const user_pointer_source& in_source, const user_pointer& in_previous);

	void draw_gui();
	float get_speed() const;
	t_float2 get_direction() const;

	bool valid = false;
	user_pointer_source source {};
	std::shared_ptr<user_pointer> previous {};

	WORD id = 0;
	POINTER_INPUT_TYPE type = 0;
	POINTER_INFO info {};

	std::shared_ptr<POINTER_PEN_INFO> pen_info {};
	std::shared_ptr<POINTER_TOUCH_INFO> touch_info {};

	t_float2 position {0.0f, 0.0f};
	t_float2 velocity {0.0f, 0.0f};
};
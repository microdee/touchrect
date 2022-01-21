
#pragma once

#include <Windows.h>
#include <variant>
#include <optional>
#include <memory>
#include "common_concepts.h"
#include <functional>

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

	bool valid = false;
	user_pointer_source source {};
	std::shared_ptr<user_pointer> previous {};

	WORD id = 0;
	POINTER_INPUT_TYPE type = 0;
	POINTER_INFO info {};

	std::shared_ptr<POINTER_PEN_INFO> pen_info {};
	std::shared_ptr<POINTER_TOUCH_INFO> touch_info {};

	std::tuple<float, float> position {0.0f, 0.0f};
	std::tuple<float, float> velocity {0.0f, 0.0f};
};
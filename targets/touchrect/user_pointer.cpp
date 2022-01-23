#include "user_pointer.h"
#include "imgui.h"
#include "fmt/format.h"

user_pointer::user_pointer(const user_pointer_source& in_source)
	: source(in_source)
	, id(GET_POINTERID_WPARAM(in_source.wParam))
	, valid(true)
{
	GetPointerType(id, &type);
	GetPointerInfo(id, &info);

	switch (type)
	{
	case PT_TOUCH:
	{
		POINTER_TOUCH_INFO touchInfo;
		if (GetPointerTouchInfo(id, &touchInfo))
			touch_info = std::make_shared<POINTER_TOUCH_INFO>(touchInfo);
	}
	break;
	case PT_PEN:
	{
		POINTER_PEN_INFO penInfo;
		if (GetPointerPenInfo(id, &penInfo))
			pen_info = std::make_shared<POINTER_PEN_INFO>(penInfo);
	}
	break;
	}

	std::get<0>(position) = static_cast<float>(info.ptPixelLocation.x);
	std::get<1>(position) = static_cast<float>(info.ptPixelLocation.y);
}

user_pointer::user_pointer(const user_pointer_source& in_source, const user_pointer& in_previous)
	: user_pointer(in_source)
{
	previous = std::make_shared<user_pointer>(in_previous);
	velocity = position - in_previous.position;
}

void user_pointer::draw_gui()
{
	namespace ui = ImGui;

	auto pos_offs = position + t_float2{ 100.0f, 100.0f };
	ui::GetForegroundDrawList()->AddLine(ui::as_vector(position), ui::as_vector(pos_offs), 0xFFFFFFFF, 2.0f);

    ui::SetNextWindowPos(ui::as_vector(pos_offs));
    ui::SetNextWindowSize({ 350.0f, 200.0f });
    ui::PushID(id);
    {
        ui::Begin(fmt::format("pointer {0}", id).c_str());
        {
            float pos[2] { std::get<0>(position), std::get<1>(position) };
            ui::DragFloat2("position", pos);

            float vel[2]{ std::get<0>(velocity), std::get<1>(velocity) };
            ui::DragFloat2("velocity", vel);
        }
        ui::End();
    }
    ui::PopID();
}

float user_pointer::get_speed() const
{
    float vel_x = std::get<0>(velocity);
    float vel_y = std::get<1>(velocity);
    return std::sqrt(vel_x * vel_x + vel_y * vel_y);
}

t_float2 user_pointer::get_direction() const
{
    auto speed = get_speed();
    return speed == 0 ? t_float2(0,0) : velocity / t_float2(speed, speed);
}

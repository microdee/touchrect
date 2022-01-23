#include "tuple_binary_operators.h"

t_float2 ImGui::as_tuple(const ImVec2 &vec)
{
    return { vec.x, vec.y };
}

t_float4 ImGui::as_tuple(const ImVec4 &vec)
{
    return { vec.x, vec.y, vec.z, vec.w };
}

ImVec2 ImGui::as_vector(const t_float2 &vec)
{
    return { std::get<0>(vec), std::get<1>(vec) };
}

ImVec4 ImGui::as_vector(const t_float4 &vec)
{
    return { std::get<0>(vec), std::get<1>(vec), std::get<2>(vec), std::get<3>(vec) };
}

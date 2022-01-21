#pragma once

#include "imgui.h"

template<IsNumber TFirst>
std::tuple<TFirst> tuple_binary_operator(
	std::tuple<TFirst> a,
	std::tuple<TFirst> b,
	std::function<TFirst(TFirst a, TFirst b)> op
) {
	return std::tuple<TFirst>(
		op(std::get<0>(a), std::get<0>(b))
	);
}

template<IsNumber TFirst, IsNumber... T>
std::tuple<TFirst, T...> tuple_binary_operator(
	std::tuple<TFirst, T...> a,
	std::tuple<TFirst, T...> b,
	std::function<TFirst(TFirst a, TFirst b)> op
) {
	auto a_tail = std::apply(
		[](TFirst h, T... tail)
		{
			return std::make_tuple(tail...);
		}, a
	);
	auto b_tail = std::apply(
		[](TFirst h, T... tail)
		{
			return std::make_tuple(tail...);
		}, b
	);

	return std::tuple_cat(
		std::tuple<TFirst>(op(std::get<0>(a), std::get<0>(b))),
		tuple_binary_operator<T...>(a_tail, b_tail, op)
	);
}

template<IsNumber... T>
std::tuple<T...> operator -(std::tuple<T...> a, std::tuple<T...> b)
{
	return tuple_binary_operator<T...>(a, b, [](auto av, auto bv) { return av - bv; });
}

template<IsNumber... T>
std::tuple<T...> operator +(std::tuple<T...> a, std::tuple<T...> b)
{
	return tuple_binary_operator<T...>(a, b, [](auto av, auto bv) { return av + bv; });
}

template<IsNumber... T>
std::tuple<T...> operator *(std::tuple<T...> a, std::tuple<T...> b)
{
	return tuple_binary_operator<T...>(a, b, [](auto av, auto bv) { return av * bv; });
}

template<IsNumber... T>
std::tuple<T...> operator /(std::tuple<T...> a, std::tuple<T...> b)
{
	return tuple_binary_operator<T...>(a, b, [](auto av, auto bv) { return av / bv; });
}

typedef std::tuple<float, float> t_float2;
typedef std::tuple<float, float, float> t_float3;
typedef std::tuple<float, float, float, float> t_float4;

namespace ImGui
{
	t_float2 as_tuple(const ImVec2& vec)
	{
		return { vec.x, vec.y };
	}
	t_float4 as_tuple(const ImVec4& vec)
	{
		return { vec.x, vec.y, vec.z, vec.w };
	}

	ImVec2 as_vector(const t_float2& vec)
	{
		return { std::get<0>(vec), std::get<1>(vec) };
	}
	ImVec4 as_vector(const t_float4& vec)
	{
		return { std::get<0>(vec), std::get<1>(vec), std::get<2>(vec), std::get<3>(vec) };
	}
}
#pragma once

#include <Windows.h>

#include <unordered_map>
#include <utility>

#include "user_pointer.h"

class pointer_injection
{
public:
	static pointer_injection singleton;

	pointer_injection();
	~pointer_injection();
	void inject(const std::unordered_map<WORD, user_pointer>& input);

private:

};
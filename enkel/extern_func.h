#pragma once

#include "value.h"

#include <functional>
#include <string>
#include <vector>

struct Extern_Func {
	std::string name;
	int min_args = 0;
	std::function<Value(const std::vector<Value>&, void* data_ptr)> callback;
};

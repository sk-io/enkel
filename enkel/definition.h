#pragma once

#include "value.h"

#include <string>

struct Scope;

// TODO: static type checking?

const int DEF_FUNC = 1 << 0;
const int DEF_CONST = 1 << 1;

struct Definition {
	std::string name;
	Value value{};
	Scope* scope = nullptr;
	int flags = 0;
};

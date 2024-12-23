#pragma once

#include "value.h"

#include <string>

struct Scope;

// TODO: const and other flags
// TODO: static types?

struct Definition {
	std::string name;
	Value value{};
	Scope* scope = nullptr;
};

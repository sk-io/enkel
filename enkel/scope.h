#pragma once

#include "value.h"
#include "definition.h"

#include <unordered_map>
#include <string>

struct GC_Obj_Instance;

struct Scope {
	Scope(Scope* _parent, GC_Obj_Instance* _this_obj) : parent(_parent), this_obj(_this_obj) {}

	Definition* find_def(const std::string& name, bool recursive = true);
	void set_def(const std::string& name, const Value& value, int flags = 0);

	// parent scope
	Scope* parent = nullptr;
	// used for methods in classes
	GC_Obj_Instance* this_obj = nullptr;
	// TODO: make sure this doesn't invalidate any pointers to defs when resized
	// should be impossible, i think?
	// How it could occur:
	// AST_Var returns pointer to some value
	// something adds a new def, causing resize and pointer invalidation
	// the original pointer is still referenced somehow?
	std::unordered_map<std::string, Definition> definitions;
};
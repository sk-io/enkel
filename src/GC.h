#pragma once

#include "Scope.h"
#include "Definition.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

enum class GC_Obj_Type {
	String,
	Array,
	Table, // TODO: dictionary?
	Instance,
};

struct GC_Obj {
	GC_Obj_Type type;
	bool reached = false;

	GC_Obj(GC_Obj_Type _type) :
		type(_type) {}
};

struct GC_Obj_String : public GC_Obj {
	std::string str;

	GC_Obj_String(const std::string& _str) :
		GC_Obj(GC_Obj_Type::String), str(_str) {}
};

struct GC_Obj_Array : public GC_Obj {
	std::vector<Value> arr;

	GC_Obj_Array() :
		GC_Obj(GC_Obj_Type::Array) {}
};

struct GC_Obj_Table : public GC_Obj {
	std::unordered_map<std::string, Definition> definitions; // properties?

	GC_Obj_Table() :
		GC_Obj(GC_Obj_Type::Table) {}
};

struct GC_Obj_Instance : public GC_Obj {
	std::string class_name;
	Scope scope; // TODO: inheritance using parent scope
	// TODO: dont copy function reference values

	GC_Obj_Instance(const Scope& _scope) :
		GC_Obj(GC_Obj_Type::Instance), scope(_scope) {}
};

class GC_Heap {
public:
	void add_obj(GC_Obj* obj);
	void garbage_collect(const Scope& scope);

private:
	void mark_obj_and_children(GC_Obj& obj);

	std::vector<std::unique_ptr<GC_Obj>> objects;
};

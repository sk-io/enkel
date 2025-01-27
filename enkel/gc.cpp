#include "gc.h"

#include <assert.h>
#include <algorithm>

void GC_Heap::add_obj(GC_Obj* obj) {
	objects.push_back(std::unique_ptr<GC_Obj>(obj));
}

void GC_Heap::garbage_collect(const Scope& scope) {
	for (auto& obj : objects) {
		obj->reached = false;
	}

	// recursively mark
	for (auto& def : scope.definitions) {
		const Value& val = def.second.value;
		if (val.type != Value_Type::GC_Obj)
			continue;

		GC_Obj* obj = (GC_Obj*) val.as.ptr;
		mark_obj_and_children(*obj);
	}

	// free unreachable objects
	objects.erase(std::remove_if(objects.begin(), objects.end(), [](const auto& obj) {
		return !obj->reached;
	}), objects.end());
}

void GC_Heap::mark_obj_and_children(GC_Obj& obj) {
	if (obj.reached)
		return;

	// mark this object
	obj.reached = true;

	// recursively mark its children, if any
	if (obj.type == GC_Obj_Type::Instance) {
		GC_Obj_Instance* instance = (GC_Obj_Instance*) &obj;

		for (auto& def : instance->scope.definitions) {
			const Value& value = def.second.value;
			if (value.type != Value_Type::GC_Obj)
				continue;

			GC_Obj* child = (GC_Obj*) value.as.ptr;
			mark_obj_and_children(*child);
		}
	} else if (obj.type == GC_Obj_Type::Array) {
		GC_Obj_Array* arr = (GC_Obj_Array*) &obj;

		for (auto& value : arr->arr) {
			if (value.type != Value_Type::GC_Obj)
				continue;

			GC_Obj* child = (GC_Obj*) value.as.ptr;
			mark_obj_and_children(*child);
		}
	} else if (obj.type == GC_Obj_Type::Table) {
		GC_Obj_Table* table = (GC_Obj_Table*) &obj;

		for (auto& def : table->definitions) {
			const Value& value = def.second.value;
			if (value.type != Value_Type::GC_Obj)
				continue;

			GC_Obj* child = (GC_Obj*) value.as.ptr;
			mark_obj_and_children(*child);
		}
	}
}

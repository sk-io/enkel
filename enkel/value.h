#pragma once

#include <stdint.h>

enum class Value_Type {
	Null,
	Num,
	Bool,
	GC_Obj,
	BC_Func_Ref,
	Func_Ref,
	Extern_Func,
};

struct GC_Obj;

struct Value {
	Value_Type type = Value_Type::Null;
	union {
		int32_t i = 0;
		float num;
		bool _bool;
		void* ptr;
	} as;

	static constexpr Value from_num(float num) {
		Value val{};
		val.type = Value_Type::Num;
		val.as.num = num;
		return val;
	}

	static constexpr Value from_bool(bool b) {
		Value val{};
		val.type = Value_Type::Bool;
		val.as._bool = b;
		return val;
	}

	static constexpr Value from_gc_obj(GC_Obj* obj) {
		Value val{};
		val.type = Value_Type::GC_Obj;
		val.as.ptr = (void*) obj;
		return val;
	}

	static constexpr Value null_value() {
		Value val{};
		val.type = Value_Type::Null;
		val.as.ptr = nullptr;
		return val;
	}
};
#pragma once

#include "Value.h"
#include "AST.h"
#include "Scope.h"
#include "Definition.h"
#include "GC.h"

#include <functional>
#include <vector>
#include <unordered_map>

enum class Control_Flow {
	Nothing,
	Return,
	Break,
	Continue,
};

struct Eval_Result {
	Value value{};
	Control_Flow cf = Control_Flow::Nothing;
	//Definition* var = nullptr;
	Value* ref = nullptr;
};

class Interpreter;

struct Extern_Func {
	std::string name;
	std::vector<Definition> args;
	std::function<Value(Interpreter& interp, const std::vector<Value>&)> callback;
};

struct Class_Decl {
	std::string name;
	std::string parent;
	Scope scope;

	Class_Decl() : scope(nullptr) {}
	//std::unordered_map<std::string, Definition> members;
};

class Interpreter {
public:
	Interpreter();

	Eval_Result eval(AST_Node* node);
	void add_external_func(const Extern_Func& callback);

	// accessors
	GC_Heap& get_heap() { return heap; }
	Scope& get_global_scope() { return global_scope; }

	std::string val_to_str(const Value& val) const;

	Value call_function(Value func_ref, const std::vector<Value>& args, GC_Obj_Instance* obj);

private:
	Eval_Result eval_node(AST_Node* node, Scope* scope, GC_Obj_Instance* selected_obj);
	void error(const std::string& msg = "") const;

	Scope global_scope;
	std::vector<Extern_Func> external_funcs;
	std::unordered_map<std::string, Class_Decl> class_decls;
	GC_Heap heap;
};
#pragma once

#include "value.h"
#include "ast.h"
#include "scope.h"
#include "definition.h"
#include "gc.h"
#include "source_info.h"
#include "extern_func.h"

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
	Value* ref = nullptr;
};

class Interpreter;

struct Class_Decl {
	std::string name;
	std::string parent;
	Scope scope;

	Class_Decl() : scope(nullptr, nullptr) {}
};

class Interpreter {
public:
	using Error_Callback_Func = std::function<void(const std::string& msg, const Source_Info* info)>;

	Interpreter();

	Eval_Result eval(AST_Node* node);
	void add_external_func(const Extern_Func& callback);

	// accessors
	GC_Heap& get_heap() { return heap; }
	Scope& get_global_scope() { return global_scope; }
	void set_error_callback(Error_Callback_Func _func) { error_callback = _func; }

	std::string get_string(const Value& val) const;
	Value call_function(Value func_ref, const std::vector<Value>& args, GC_Obj_Instance* obj = nullptr, AST_Node* node = nullptr);
	Value create_string(const std::string& str);
	const Value& expect_value(const Value& val, Value_Type expected_type, const AST_Node* node) const;

	// temporary??
	AST_Node* extern_func_node = nullptr; // set when calling extern func to pass info
private:
	Eval_Result eval_node(AST_Node* node, Scope* scope, GC_Obj_Instance* selected_obj = nullptr);
	void error(const std::string& msg = "", const AST_Node* node = nullptr) const;

	Error_Callback_Func error_callback = nullptr;
	Scope global_scope;
	std::vector<Extern_Func> external_funcs;
	std::unordered_map<std::string, Class_Decl> class_decls;
	GC_Heap heap;
};
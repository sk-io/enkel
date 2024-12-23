#include "Interpreter.h"

#include <assert.h>
#include <iostream>

Interpreter::Interpreter(Error_Callback_Func _error_callback) :
	global_scope(nullptr, nullptr), error_callback(_error_callback) {
	// typeof(value)
	add_external_func({"typeof", 1, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		const Value& val = args[0];

		std::string result;
		switch (val.type) {
		case Value_Type::Null:
			result = "null";
			break;
		case Value_Type::Num:
			result = "number";
			break;
		case Value_Type::Bool:
			result = "boolean";
			break;
		case Value_Type::Func_Ref:
		case Value_Type::Extern_Func:
			result = "function";
			break;
		case Value_Type::GC_Obj: {
			GC_Obj* gc_obj = (GC_Obj*) val.as.ptr;
			switch (gc_obj->type) {
			case GC_Obj_Type::Array:
				result = "array";
				break;
			case GC_Obj_Type::String:
				result = "string";
				break;
			case GC_Obj_Type::Instance: {
				GC_Obj_Instance* instance = (GC_Obj_Instance*) gc_obj;

				result = instance->class_name;
				break;
			}
			}
			break;
		}
		}

		return create_string(result);
	}});

	// _run_gc()  [temporary]
	add_external_func({"_run_gc", 0, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		// TODO: run from current scope???
		heap.garbage_collect(global_scope);
		return {};
	}});

	// print(value)
	add_external_func({"print", 1, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		std::cout << "print(): " << interp.get_string(args[0]) << "\n";
		return {};
	}});

	// min(value)
	add_external_func({"min", 2, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num || args[1].type != Value_Type::Num) {
			error("expected numbers");
		}
		return Value::from_num(std::min(args[0].as.num, args[1].as.num));
	}});

	// max(value)
	add_external_func({"max", 2, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num || args[1].type != Value_Type::Num) {
			error("expected numbers");
		}
		return Value::from_num(std::max(args[0].as.num, args[1].as.num));
	}});

	// floor(value)
	add_external_func({"floor", 1, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num) {
			error("expected number argument");
		}
		
		return Value::from_num(std::floor(args[0].as.num));
	}});

	// ceil(value)
	add_external_func({"ceil", 1, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num) {
			error("expected number argument");
		}

		return Value::from_num(std::ceil(args[0].as.num));
	}});

	// lerp(a, b, ratio)
	add_external_func({"lerp", 3, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num ||
			args[1].type != Value_Type::Num ||
			args[2].type != Value_Type::Num) {
			error("expected 3 numbers");
		}
		float diff = args[1].as.num - args[0].as.num;

		return Value::from_num(args[0].as.num + diff * args[2].as.num);
	}});

	// wrap(min, max, value)
	// min is inclusive, max is exclusive
	// returns value wrapped around [min, max]
	// example: wrap(0, 10, -1) returns 9
	add_external_func({"wrap", 3, [this](Interpreter& interp, const std::vector<Value>& args) -> Value {
		if (args[0].type != Value_Type::Num ||
			args[1].type != Value_Type::Num ||
			args[2].type != Value_Type::Num) {
			error("expected 3 numbers");
		}

		return Value::from_num(std::ceil(args[0].as.num));
	}});
}

Eval_Result Interpreter::eval(AST_Node* node) {
	return eval_node(node, &global_scope, nullptr);
}

void Interpreter::add_external_func(const Extern_Func& func) {
	// TODO: duplicate check

	int id = external_funcs.size();
	external_funcs.push_back(func);

	Value val;
	val.type = Value_Type::Extern_Func;
	val.as.i = id;
	global_scope.set_def(func.name, val);
}

std::string Interpreter::get_string(const Value& val) const {
	switch (val.type) {
	case Value_Type::Null: return "null";
	case Value_Type::Num: return std::to_string(val.as.num);
	case Value_Type::Bool: return val.as._bool ? "true" : "false";
	case Value_Type::Func_Ref: return "func_ref";
	case Value_Type::GC_Obj: {
		GC_Obj* obj = (GC_Obj*) val.as.ptr;

		switch (obj->type) {
		case GC_Obj_Type::String: return ((GC_Obj_String*) obj)->str;
		case GC_Obj_Type::Array: {
			GC_Obj_Array* arr = (GC_Obj_Array*) obj;

			std::string str = "[";
			for (size_t i = 0; i < arr->arr.size(); i++) {
				const Value& item = arr->arr[i];

				str += get_string(item);
				if (i != arr->arr.size() - 1)
					str += ", ";
			}
			str += "]";
			return str;
		}
		case GC_Obj_Type::Instance: {
			GC_Obj_Instance* instance = (GC_Obj_Instance*) obj;
			// TODO: print members
			return instance->class_name;
		}
		}
		error();
	}
	}
	error();
	return std::string();
}

Value Interpreter::call_function(Value func_ref, const std::vector<Value>& args, GC_Obj_Instance* obj) {
	// check if it's an external c++ function
	if (func_ref.type == Value_Type::Extern_Func) {
		const Extern_Func& func = external_funcs[func_ref.as.i];
		if (args.size() < func.min_args) {
			error("too few arguments");
		}

		return func.callback(*this, args);
	}

	AST_Func_Decl* func_decl = (AST_Func_Decl*) func_ref.as.ptr;
	if (args.size() != func_decl->args.size()) {
		error("incorrect number of arguments");
	}

	Scope func_scope((obj != nullptr) ? &obj->scope : &global_scope, obj);
	if (func_decl->is_global) {
		func_scope.parent = &global_scope;
		func_scope.this_obj = nullptr;
	}

	// put evaluated args in callee scope
	for (int i = 0; i < func_decl->args.size(); i++) {
		func_scope.set_def(func_decl->args[i].name, args[i]);
	}

	Eval_Result call_result = eval_node(func_decl->body.get(), &func_scope);
	return call_result.value;
}

Value Interpreter::create_string(const std::string& str) {
	GC_Obj_String* obj = new GC_Obj_String(str);
	heap.add_obj(obj);

	Value val;
	val.type = Value_Type::GC_Obj;
	val.as.ptr = (void*) obj;
	return val;
}

// a.b()
// selected_obj is set for children evals calls when using dot operator
Eval_Result Interpreter::eval_node(AST_Node* node, Scope* scope, GC_Obj_Instance* selected_obj) {
	switch (node->type) {
	case AST_Node_Type::Literal: {
		AST_Literal* sub = (AST_Literal*) node;

		return {sub->val};
	}
	case AST_Node_Type::String_Literal: {
		AST_String_Literal* sub = (AST_String_Literal*) node;

		return {create_string(sub->str)};
	}
	case AST_Node_Type::Unary_Op: {
		AST_Unary_Op* sub = (AST_Unary_Op*) node;

		Eval_Result expr_eval = eval_node(sub->expr.get(), scope);

		if (expr_eval.ref == nullptr) {
			error("expression is not modifiable");
		}

		if (expr_eval.ref->type != Value_Type::Num) {
			error("expected number");
		}

		Value old_value = expr_eval.value;

		switch (sub->op) {
		case Unary_Op::Increment:
			*expr_eval.ref = Value::from_num(expr_eval.ref->as.num + 1);
			break;
		case Unary_Op::Decrement:
			*expr_eval.ref = Value::from_num(expr_eval.ref->as.num - 1);
			break;
		default:
			error();
		}

		return {old_value};
	}
	case AST_Node_Type::Bin_Op: {
		AST_Bin_Op* sub = (AST_Bin_Op*) node;

		if (sub->op == Bin_Op::Assign) {
			// NOTE: evaluate right side first, since it can cause container resizes
			// and create memory corruption

			Value rval = eval_node(sub->right.get(), scope).value;

			Value* ref = eval_node(sub->left.get(), scope).ref;
			assert(ref != nullptr);

			*ref = rval;

			Eval_Result result;
			result.value = rval;
			result.ref = ref;
			return result;
		}

		if (sub->op == Bin_Op::Dot) {
			// dot operator needs to not immediately change scope,
			// but store the scope and change when rightside function call happens
			Value lval = eval_node(sub->left.get(), scope).value;
			if (lval.type != Value_Type::GC_Obj) {
				error("expected class instance");
			}

			GC_Obj* gc_obj = (GC_Obj*) lval.as.ptr;

			// TODO: ability to register methods to types?
			// or just, declare classes externally?
			// 
			// array.length
			if (gc_obj->type == GC_Obj_Type::Array && sub->right->type == AST_Node_Type::Var) {
				GC_Obj_Array* arr = (GC_Obj_Array*) gc_obj;
				AST_Var* var = (AST_Var*) sub->right.get();
				if (var->name == "length") {
					return {Value::from_num(arr->arr.size())};
				}
			}

			// array methods
			if (gc_obj->type == GC_Obj_Type::Array && sub->right->type == AST_Node_Type::Func_Call) {
				GC_Obj_Array* arr = (GC_Obj_Array*) gc_obj;

				AST_Func_Call* fcall = (AST_Func_Call*) sub->right.get();
				if (fcall->expr->type != AST_Node_Type::Var) {
					error();
				}

				AST_Var* var = (AST_Var*) fcall->expr.get();

				// array.push(val)
				if (var->name == "push") {
					if (fcall->args.size() != 1) {
						error("incorrect number of args");
					}

					arr->arr.push_back(eval_node(fcall->args[0].get(), scope).value);
					return {};
				}

				// array.pop()
				if (var->name == "pop") {
					if (fcall->args.size() != 0) {
						error("incorrect number of args");
					}

					Value val = arr->arr.back();
					arr->arr.pop_back();
					return {val};
				}

				if (var->name == "remove_at") {
					if (fcall->args.size() != 1) {
						error("incorrect number of args");
					}

					Value index_val = eval_node(fcall->args[0].get(), scope).value;
					if (index_val.type != Value_Type::Num) {
						error("expected integer index");
					}

					int index = (int) index_val.as.num;
					if (index < 0 || index >= arr->arr.size()) {
						error("index is out of bounds");
					}

					Value removed_val = arr->arr[index];
					arr->arr.erase(arr->arr.begin() + index);

					return {removed_val};
				}
			}

			if (gc_obj->type != GC_Obj_Type::Instance) {
				error("expected class instance");
			}

			GC_Obj_Instance* instance = (GC_Obj_Instance*) gc_obj;
			return eval_node(sub->right.get(), scope, instance);
		}
		
		if (sub->op == Bin_Op::Is) {
			Value lval = eval_node(sub->left.get(), scope).value;

			if (lval.type != Value_Type::GC_Obj) {
				return {Value::from_bool(false)};
			}

			GC_Obj* gc_obj = (GC_Obj*) lval.as.ptr;
			if (gc_obj->type != GC_Obj_Type::Instance) {
				// TODO: x is String?
				return {Value::from_bool(false)};
			}

			if (sub->right->type != AST_Node_Type::Var) {
				error("expected type name");
			}

			AST_Var* compare = (AST_Var*) sub->right.get();
			GC_Obj_Instance* inst = (GC_Obj_Instance*) gc_obj;

			// TODO: check parent class
			return {Value::from_bool(inst->class_name == compare->name)};
		}

		Eval_Result l_eval = eval_node(sub->left.get(), scope);
		Eval_Result r_eval = eval_node(sub->right.get(), scope);

		Value lval = l_eval.value;
		Value rval = r_eval.value;

		if (lval.type == Value_Type::Num && rval.type == Value_Type::Num) {
			Value val;
			switch (sub->op) {
			case Bin_Op::Add:
				val = Value::from_num(lval.as.num + rval.as.num);
				break;
			case Bin_Op::Sub:
				val = Value::from_num(lval.as.num - rval.as.num);
				break;
			case Bin_Op::Mul:
				val = Value::from_num(lval.as.num * rval.as.num);
				break;
			case Bin_Op::Div:
				val = Value::from_num(lval.as.num / rval.as.num);
				break;
			case Bin_Op::Equals:
				val = Value::from_bool(lval.as.num == rval.as.num);
				break;
			case Bin_Op::Not_Equals:
				val = Value::from_bool(lval.as.num != rval.as.num);
				break;
			case Bin_Op::Greater_Than:
				val = Value::from_bool(lval.as.num > rval.as.num);
				break;
			case Bin_Op::Greater_Than_Equals:
				val = Value::from_bool(lval.as.num >= rval.as.num);
				break;
			case Bin_Op::Less_Than:
				val = Value::from_bool(lval.as.num < rval.as.num);
				break;
			case Bin_Op::Less_Than_Equals:
				val = Value::from_bool(lval.as.num <= rval.as.num);
				break;
			case Bin_Op::Add_Assign:
				val = Value::from_num(lval.as.num + rval.as.num);
				*l_eval.ref = val;
				break;
			case Bin_Op::Sub_Assign:
				val = Value::from_num(lval.as.num - rval.as.num);
				*l_eval.ref = val;
				break;
			case Bin_Op::Mul_Assign:
				val = Value::from_num(lval.as.num * rval.as.num);
				*l_eval.ref = val;
				break;
			case Bin_Op::Div_Assign:
				val = Value::from_num(lval.as.num / rval.as.num);
				*l_eval.ref = val;
				break;
			default:
				error();
			}

			Eval_Result result;
			result.value = val;
			return result;
		}

		if (lval.type == Value_Type::GC_Obj && rval.type == Value_Type::GC_Obj) {
			GC_Obj* lobj = (GC_Obj*) lval.as.ptr;
			GC_Obj* robj = (GC_Obj*) rval.as.ptr;

			if (lobj->type == GC_Obj_Type::String && robj->type == GC_Obj_Type::String) {

				GC_Obj_String* lstr = (GC_Obj_String*) lval.as.ptr;
				GC_Obj_String* rstr = (GC_Obj_String*) rval.as.ptr;

				if (sub->op == Bin_Op::Add) {
					GC_Obj_String* obj = new GC_Obj_String(lstr->str + rstr->str);
					heap.add_obj(obj);

					Eval_Result result;
					result.value = Value::from_gc_obj(obj);
					return result;
				}

				if (sub->op == Bin_Op::Equals) {
					Eval_Result result;
					result.value = Value::from_bool(lstr->str == rstr->str);
					return result;
				}

				if (sub->op == Bin_Op::Not_Equals) {
					Eval_Result result;
					result.value = Value::from_bool(lstr->str != rstr->str);
					return result;
				}
			}
		}

		if (lval.type == Value_Type::Null || rval.type == Value_Type::Null) {
			if (sub->op == Bin_Op::Equals) {
				return {Value::from_bool(lval.type == rval.type)};
			}

			if (sub->op == Bin_Op::Not_Equals) {
				return {Value::from_bool(lval.type != rval.type)};
			}
		}

		assert(false);
		return {};
	}
	case AST_Node_Type::Block: {
		AST_Block* sub = (AST_Block*) node;
		for (auto& statement : sub->statements) {
			auto result = eval_node(statement.get(), scope);

			if (result.cf != Control_Flow::Nothing)
				return result;
		}
		break;
	}
	case AST_Node_Type::Var_Decl: {
		AST_Var_Decl* sub = (AST_Var_Decl*) node;
		if (scope->find_def(sub->name) != nullptr) {
			error("duplicate definition name: " + sub->name);
		}

		Value val = Value::null_value();
		if (sub->init != nullptr) {
			val = eval_node(sub->init.get(), scope).value;
		}

		scope->set_def(sub->name, val);
		break;
	}
	case AST_Node_Type::Var: {
		AST_Var* sub = (AST_Var*) node;
		
		Definition* var = nullptr;

		if (selected_obj != nullptr) {
			// if foo.bar, search the scope of foo
			var = selected_obj->scope.find_def(sub->name, false);
		} else {
			// search current scope recursively
			var = scope->find_def(sub->name);
		}

		if (var == nullptr) {
			error("no such variable: " + sub->name);
		}

		Eval_Result ret;
		ret.value = var->value;
		ret.ref = &var->value;
		return ret;
	}
	case AST_Node_Type::Func_Decl: {
		AST_Func_Decl* sub = (AST_Func_Decl*) node;
		if (scope->find_def(sub->name) != nullptr) {
			error("duplicate definition name: " + sub->name);
		}

		Value val;
		val.type = Value_Type::Func_Ref;
		val.as.ptr = (void*) sub;

		scope->set_def(sub->name, val);
		break;
	}
	case AST_Node_Type::Return: {
		AST_Return* sub = (AST_Return*) node;

		Value ret_val = eval_node(sub->expr.get(), scope).value;
		
		Eval_Result result;
		result.cf = Control_Flow::Return;
		result.value = ret_val;
		return result;
	}
	case AST_Node_Type::Func_Call: {
		AST_Func_Call* sub = (AST_Func_Call*) node;

		// foo.bar(); foo is selected_obj
		Value func_ref = eval_node(sub->expr.get(), scope, selected_obj).value;
		if (func_ref.type != Value_Type::Func_Ref && func_ref.type != Value_Type::Extern_Func) {
			error("no such function");
		}

		// evaluate caller argument expressions
		std::vector<Value> arg_evals;
		for (auto& arg : sub->args) {
			arg_evals.push_back(eval_node(arg.get(), scope).value);
		}
		
		Value result = call_function(func_ref, arg_evals, selected_obj != nullptr ? selected_obj : scope->this_obj);
		return {result};
	}
	case AST_Node_Type::If: {
		AST_If* sub = (AST_If*) node;
		
		Value cond_val = eval_node(sub->condition.get(), scope).value;

		if (cond_val.type != Value_Type::Bool) {
			error("expected bool");
		}

		if (cond_val.as._bool) {
			Scope new_scope(scope, scope->this_obj);
			return eval_node(sub->if_body.get(), &new_scope);
		} else if (sub->else_body != nullptr) {
			Scope new_scope(scope, scope->this_obj);
			return eval_node(sub->else_body.get(), &new_scope);
		}
		
		return {};
	}
	case AST_Node_Type::While: {
		AST_While* sub = (AST_While*) node;

		Scope new_scope(scope, scope->this_obj);
		while (true) {
			Value cond_val = eval_node(sub->condition.get(), scope).value;

			if (cond_val.type != Value_Type::Bool) {
				error("expected bool");
			}

			if (!cond_val.as._bool) {
				break;
			}

			const auto& body_result = eval_node(sub->body.get(), &new_scope);

			if (body_result.cf == Control_Flow::Return)
				return body_result;
			if (body_result.cf == Control_Flow::Break)
				break;
			// on continue, do nothing
		}

		return {};
	}
	case AST_Node_Type::For: {
		AST_For* sub = (AST_For*) node;

		Scope new_scope(scope, scope->this_obj);

		Value expr_val = eval_node(sub->expr.get(), scope).value;

		if (expr_val.type == Value_Type::Num) {
			int count = (int) expr_val.as.num;
			int i = 0;

			while (i < count) {
				new_scope.set_def(sub->var_name, Value::from_num(i));

				const auto& body_result = eval_node(sub->body.get(), &new_scope, nullptr);

				if (body_result.cf == Control_Flow::Return)
					return body_result;
				if (body_result.cf == Control_Flow::Break)
					break;
				// on continue, do nothing
				i++;
			}

			return {};
		}

		if (expr_val.type == Value_Type::GC_Obj) {
			GC_Obj* gc_obj = (GC_Obj*) expr_val.as.ptr;

			if (gc_obj->type == GC_Obj_Type::Array) {
				GC_Obj_Array* arr = (GC_Obj_Array*) gc_obj;

				int i = 0;

				while (i < arr->arr.size()) {
					new_scope.set_def(sub->var_name, arr->arr[i]);

					const auto& body_result = eval_node(sub->body.get(), &new_scope, nullptr);

					if (body_result.cf == Control_Flow::Return)
						return body_result;
					if (body_result.cf == Control_Flow::Break)
						break;
					// on continue, do nothing
					i++;
				}

				return {};
			}
		}

		error("cant iterate over that");
		return {};
	}
	case AST_Node_Type::Array_Init: {
		AST_Array_Init* sub = (AST_Array_Init*) node;

		GC_Obj_Array* gc_obj = new GC_Obj_Array();
		heap.add_obj(gc_obj);

		for (auto& item : sub->items) {
			gc_obj->arr.push_back(eval_node(item.get(), scope).value);
		}

		Value val;
		val.type = Value_Type::GC_Obj;
		val.as.ptr = (void*) gc_obj;

		return {val};
	}
	case AST_Node_Type::Subscript: {
		AST_Subscript* sub = (AST_Subscript*) node;

		Value expr_val = eval_node(sub->expr.get(), scope).value;
		if (expr_val.type != Value_Type::GC_Obj) {
			error("expected gc obj");
		}

		GC_Obj* gc_obj = (GC_Obj*) expr_val.as.ptr;
		if (gc_obj->type != GC_Obj_Type::Array) {
			error("i can only subscript arrays man");
		}

		Value subscript_val = eval_node(sub->subscript.get(), scope).value;
		if (subscript_val.type != Value_Type::Num) {
			error("not a number");
		}

		int index = (int) subscript_val.as.num;

		GC_Obj_Array* arr = (GC_Obj_Array*) gc_obj;

		if (index < 0 || index >= arr->arr.size()) {
			error("out of bounds");
		}

		Eval_Result result;
		result.ref = &arr->arr[index];
		result.value = arr->arr[index];
		return result;
	}
	case AST_Node_Type::Class_Decl: {
		AST_Class_Decl* sub = (AST_Class_Decl*) node;
		
		Class_Decl decl;
		decl.name = sub->name;
		decl.parent = sub->parent;

		for (const auto& member : sub->members) {
			eval_node(member.get(), &decl.scope);
		}

		if (class_decls.find(decl.name) != class_decls.end()) {
			error("redefinition of class " + decl.name);
		}

		class_decls[decl.name] = decl;
		return {};
	}
	case AST_Node_Type::New: {
		AST_New* sub = (AST_New*) node;

		if (class_decls.find(sub->name) == class_decls.end()) {
			error("class not found: " + sub->name);
		}

		const Class_Decl& class_decl = class_decls[sub->name];

		// TODO: this_obj???
		GC_Obj_Instance* instance = new GC_Obj_Instance(Scope(&global_scope, nullptr));
		heap.add_obj(instance);

		instance->class_name = sub->name;
		instance->scope.definitions = class_decl.scope.definitions;

		std::string cur = class_decl.parent;
		while (!cur.empty()) {
			if (class_decls.find(cur) == class_decls.end()) {
				error("class not found: " + cur);
			}

			for (const auto& it : class_decls[cur].scope.definitions) {
				const std::string& def_name = it.first;

				if (instance->scope.find_def(def_name, false) != nullptr) {
					// TODO: proper error handling, cant overload a var with a func etc.
					//error("duplicate variable name: " + def_name);
					continue;
				}

				instance->scope.set_def(def_name, it.second.value);
			}

			cur = class_decls[cur].parent;
		}

		Definition* constructor = instance->scope.find_def("init", false);
		if (constructor != nullptr) {
			// evaluate constructor args
			std::vector<Value> arg_evals;
			for (auto& arg : sub->args) {
				arg_evals.push_back(eval_node(arg.get(), scope, nullptr).value);
			}

			// call constructor
			call_function(constructor->value, arg_evals, instance);
		}

		if (constructor == nullptr && !sub->args.empty()) {
			error("default constructor takes no args");
		}

		return {Value::from_gc_obj(instance)};
	}
	case AST_Node_Type::This: {
		if (scope->this_obj == nullptr) {
			error("not in a class");
		}

		Value val = Value::from_gc_obj((GC_Obj*) scope->this_obj);
		Eval_Result result;
		result.value = val;
		result.ref = &val;
		return result;
	}
	case AST_Node_Type::Break: {
		Eval_Result result;
		result.cf = Control_Flow::Break;
		return result;
	}
	case AST_Node_Type::Continue: {
		Eval_Result result;
		result.cf = Control_Flow::Continue;
		return result;
	}
	case AST_Node_Type::Null: {
		return {Value::null_value()};
	}
	default:
		error("unhandled node type");
	}

	return {};
}

void Interpreter::error(const std::string& msg) const {
	if (error_callback != nullptr) {
		error_callback(msg);
	} else {
		std::cout << "Interpreter error: " << msg << "\n";
		assert(false);
		exit(1);
	}
}

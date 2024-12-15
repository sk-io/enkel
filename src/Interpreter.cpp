#include "Interpreter.h"

#include <assert.h>
#include <iostream>

Interpreter::Interpreter() {
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

std::string Interpreter::val_to_str(const Value& val) const {
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

				str += val_to_str(item);
				if (i != arr->arr.size() - 1)
					str += ", ";
			}
			str += "]";
			return str;
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
		if (args.size() != func.args.size()) {
			error("incorrect number of arguments");
		}

		return func.callback(*this, args);
	}

	AST_Func_Decl* func_decl = (AST_Func_Decl*) func_ref.as.ptr;
	if (args.size() != func_decl->args.size()) {
		error("incorrect number of arguments");
	}

	Scope func_scope;
	func_scope.parent = (obj != nullptr) ? &obj->scope : &global_scope;
	func_scope.this_obj = obj;

	// put evaluated args in callee scope
	for (int i = 0; i < func_decl->args.size(); i++) {
		func_scope.set_def(func_decl->args[i].name, args[i]);
	}

	Eval_Result call_result = eval_node(func_decl->body.get(), &func_scope, nullptr);
	return call_result.value;
}

// selected_obj is set for children evals calls when using dot operator
Eval_Result Interpreter::eval_node(AST_Node* node, Scope* scope, GC_Obj_Instance* selected_obj) {
	switch (node->type) {
	case AST_Node_Type::Literal: {
		AST_Literal* sub = (AST_Literal*) node;
		return {sub->val};
	}
	case AST_Node_Type::String_Literal: {
		AST_String_Literal* sub = (AST_String_Literal*) node;

		GC_Obj_String* obj = new GC_Obj_String(sub->str);
		heap.add_obj(obj);

		Value val;
		val.type = Value_Type::GC_Obj;
		val.as.ptr = (void*) obj;

		return {val};
	}
	case AST_Node_Type::Bin_Op: {
		AST_Bin_Op* sub = (AST_Bin_Op*) node;

		if (sub->op == Bin_Op::Assign) {
			// NOTE: evaluate right side first, since it can cause container resizes
			// and create memory corruption

			Value rval = eval_node(sub->right.get(), scope, selected_obj).value;

			Value* ref = eval_node(sub->left.get(), scope, selected_obj).ref;
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
			Value lval = eval_node(sub->left.get(), scope, selected_obj).value;
			if (lval.type != Value_Type::GC_Obj) {
				error("expected class instance");
			}

			GC_Obj* gc_obj = (GC_Obj*) lval.as.ptr;

			// array.length hack
			if (gc_obj->type == GC_Obj_Type::Array && sub->right->type == AST_Node_Type::Var) {
				GC_Obj_Array* arr = (GC_Obj_Array*) gc_obj;
				AST_Var* var = (AST_Var*) sub->right.get();
				if (var->name == "length") {
					return {Value::from_num(arr->arr.size())};
				}
			}

			if (gc_obj->type != GC_Obj_Type::Instance) {
				error("expected class instance");
			}

			GC_Obj_Instance* instance = (GC_Obj_Instance*) gc_obj;
			/*
			if (sub->right->type == AST_Node_Type::Var) {
				AST_Var* var = (AST_Var*) sub->right.get();

				const Definition& def = *instance->scope.find_def(var->name);

				return {def.value};
			} else if (sub->right->type == AST_Node_Type::Func_Call) {
				// TODO: do this properly
				// allow for thing.list[2](); etc..
				AST_Func_Call* func_call = (AST_Func_Call*) sub->right.get();
				assert(func_call->expr->type == AST_Node_Type::Var);
				AST_Var* var = (AST_Var*) func_call->expr.get();

				const Definition& def = *instance->scope.find_def(var->name);
				const Value& func_ref = def.value;

				// evaluate caller argument expressions
				std::vector<Value> arg_evals;
				for (auto& arg : func_call->args) {
					arg_evals.push_back(eval_node(arg.get(), scope, selected_obj).value);
				}

				assert(func_ref.type != Value_Type::Extern_Func);

				AST_Func_Decl* func_decl = (AST_Func_Decl*) func_ref.as.ptr;
				if (func_call->args.size() != func_decl->args.size()) {
					error("incorrect number of arguments");
				}

				Scope func_scope(&instance->scope);

				func_scope.set_def("this", lval);

				// put evaluated args in callee scope
				for (int i = 0; i < func_decl->args.size(); i++) {
					func_scope.set_def(func_decl->args[i].name, arg_evals[i]);
				}

				Eval_Result call_result = eval_node(func_decl->body.get(), &func_scope, selected_obj);

				Eval_Result result;
				result.ref = call_result.ref;
				result.value = call_result.value;
				return result;
			} else {
				error("expected var access or func call");
			}
			*/

			return eval_node(sub->right.get(), scope, instance);
		}

		Value lval = eval_node(sub->left.get(), scope, selected_obj).value;
		Value rval = eval_node(sub->right.get(), scope, selected_obj).value;

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

		assert(false);
		return {};
	}
	case AST_Node_Type::Block: {
		AST_Block* sub = (AST_Block*) node;
		for (auto& statement : sub->statements) {
			auto result = eval_node(statement.get(), scope, selected_obj);

			if (result.cf != Control_Flow::Nothing)
				return result;
		}
		break;
	}
	case AST_Node_Type::Var_Decl: {
		AST_Var_Decl* sub = (AST_Var_Decl*) node;
		if (scope->find_def(sub->name) != nullptr) {
			error("duplicate variable name");
		}

		Value val = Value::null_value();
		if (sub->init != nullptr) {
			val = eval_node(sub->init.get(), scope, selected_obj).value;
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
			// search global scope
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
			error("duplicate variable name");
		}

		Value val;
		val.type = Value_Type::Func_Ref;
		val.as.ptr = (void*) sub;

		scope->set_def(sub->name, val);
		break;
	}
	case AST_Node_Type::Return: {
		AST_Return* sub = (AST_Return*) node;

		Value ret_val = eval_node(sub->expr.get(), scope, selected_obj).value;
		
		Eval_Result result;
		result.cf = Control_Flow::Return;
		result.value = ret_val;
		return result;
	}
	case AST_Node_Type::Func_Call: {
		AST_Func_Call* sub = (AST_Func_Call*) node;

		Value func_ref = eval_node(sub->expr.get(), scope, selected_obj).value;
		if (func_ref.type != Value_Type::Func_Ref && func_ref.type != Value_Type::Extern_Func) {
			error("no such function");
		}

		// evaluate caller argument expressions
		std::vector<Value> arg_evals;
		for (auto& arg : sub->args) {
			arg_evals.push_back(eval_node(arg.get(), scope, selected_obj).value);
		}

		Value result = call_function(func_ref, arg_evals, selected_obj);
		return {result};
	}
	case AST_Node_Type::If: {
		AST_Conditional* sub = (AST_Conditional*) node;
		
		Value cond_val = eval_node(sub->condition.get(), scope, selected_obj).value;

		if (cond_val.type != Value_Type::Bool) {
			error("expected bool");
		}

		if (cond_val.as._bool) {
			Scope new_scope(scope);
			eval_node(sub->body.get(), &new_scope, selected_obj);
		}
		
		return {};
	}
	case AST_Node_Type::While: {
		AST_Conditional* sub = (AST_Conditional*) node;

		Scope new_scope(scope);
		while (true) {
			Value cond_val = eval_node(sub->condition.get(), scope, selected_obj).value;

			if (cond_val.type != Value_Type::Bool) {
				error("expected bool");
			}

			if (!cond_val.as._bool) {
				break;
			}

			eval_node(sub->body.get(), &new_scope, selected_obj);
		}

		return {};
	}
	case AST_Node_Type::For: {
		AST_For* sub = (AST_For*) node;

		Scope new_scope(scope);

		Value expr_val = eval_node(sub->expr.get(), scope, selected_obj).value;

		if (expr_val.type == Value_Type::Num) {
			int count = (int) expr_val.as.num;
			int i = 0;

			while (i < count) {
				new_scope.set_def(sub->var_name, Value::from_num(i));

				eval_node(sub->body.get(), &new_scope, nullptr);

				// TODO: control flow
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

					eval_node(sub->body.get(), &new_scope, nullptr);

					// TODO: control flow
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
			gc_obj->arr.push_back(eval_node(item.get(), scope, selected_obj).value);
		}

		Value val;
		val.type = Value_Type::GC_Obj;
		val.as.ptr = (void*) gc_obj;

		return {val};
	}
	case AST_Node_Type::Subscript: {
		AST_Subscript* sub = (AST_Subscript*) node;

		Value expr_val = eval_node(sub->expr.get(), scope, selected_obj).value;
		if (expr_val.type != Value_Type::GC_Obj) {
			error("expected gc obj");
		}

		GC_Obj* gc_obj = (GC_Obj*) expr_val.as.ptr;
		if (gc_obj->type != GC_Obj_Type::Array) {
			error("i can only subscript arrays man");
		}

		Value subscript_val = eval_node(sub->subscript.get(), scope, selected_obj).value;
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

		for (const auto& member : sub->members) {
			eval_node(member.get(), &decl.scope, selected_obj);
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

		GC_Obj_Instance* instance = new GC_Obj_Instance(Scope(&global_scope));
		heap.add_obj(instance);

		instance->class_name = sub->name;
		instance->scope.definitions = class_decl.scope.definitions;

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
	default:
		error();
	}

	return {};
}

void Interpreter::error(const std::string& msg) const {
	std::cout << "Interpreter error: " << msg << "\n";
	assert(false);
	exit(1);
}

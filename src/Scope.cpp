#include "Scope.h"

Definition* Scope::find_def(const std::string& name, bool recursive) {
    if (definitions.find(name) != definitions.end()) {
        return &definitions[name];
    }

    if (recursive && parent != nullptr) {
        return parent->find_def(name, true);
    }

    return nullptr;
}

void Scope::set_def(const std::string& name, const Value& value) {
    Definition def;
    def.name = name;
    def.value = value;
    def.scope = this;
    definitions[name] = def;
}

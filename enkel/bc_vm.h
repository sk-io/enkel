#pragma once

#include "value.h"
#include "bc.h"

#include <vector>
#include <stdint.h>
#include <string>

struct BC_Frame {
	uint32_t start;
	uint32_t num_vars;
};

class BC_VM {
public:
	void run(const BC_Program* program);

	uint8_t eat_u8();
	uint16_t eat_u16();
	uint32_t eat_u32();
	float eat_float();
private:
	const BC_Program* program = nullptr;

	uint32_t pos;

	std::vector<Value> op_stack;
	std::vector<uint32_t> call_stack;
	std::vector<BC_Frame> frame_stack;
	std::vector<Value> var_stack;
};

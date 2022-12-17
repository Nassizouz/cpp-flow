#pragma once

#include <utility>

#define FLOW_OP __flow_op
#define FLOW_OP_T __flow_op_T
#define FLOW_ACTION template <typename FLOW_OP_T> decltype(auto) operator>> (FLOW_OP_T&& FLOW_OP)
#define FLOW_CONTINUE(...) return std::forward<FLOW_OP_T>(FLOW_OP)(__VA_ARGS__)
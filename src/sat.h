#include "libr.h"
#include "structures.h"
#include "algo.h"

// cnf_parsing.cpp
std::vector<std::vector<int>> read_cnf(const std::string& file_name, size_t& variable_amount, size_t& clause_amount);

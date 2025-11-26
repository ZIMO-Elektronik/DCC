#pragma once

#include <string>
#include "state.hpp"

void query(State& state);
void from_query(State& state, std::string const& url);
std::string to_query(State& state);

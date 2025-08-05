#pragma once

#include <string>
#include <vector>

std::string exec(const char *cmd);
std::vector<std::vector<std::string>> read_csv(const std::string &filename);

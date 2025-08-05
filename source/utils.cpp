#include "../headers/utils.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

std::string exec(const char *cmd) {
  std::array<char, 128> buffer{};
  std::string result;
  FILE *pipe = popen(cmd, "r");
  if (!pipe)
    return "";

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }

  auto returnCode = pclose(pipe);
  if (returnCode != 0) {
    return "";
  }

  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }
  return result;
}

std::vector<std::vector<std::string>> read_csv(const std::string &filename) {
  std::vector<std::vector<std::string>> data;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open the file " << filename << std::endl;
    return data;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> row;
    std::stringstream ss(line);
    std::string value;
    while (std::getline(ss, value, ',')) {
      row.push_back(value);
    }
    data.push_back(row);
  }
  file.close();
  return data;
}

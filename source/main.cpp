#include "app.hpp"

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

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "com.nian.loki");
  Loki window;

  return app->run(window);
}


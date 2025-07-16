#include "app.hpp"

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "com.nian.loki");
  Loki window;

  return app->run(window);
}


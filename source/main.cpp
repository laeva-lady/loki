#include "app.hpp"
#include <libayatana-appindicator/app-indicator.h>

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "com.nian.loki");
  Loki window(app);

  // Hide the window initially
  window.hide();

  return app->run(window);
}


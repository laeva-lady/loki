#include "gtkmm/button.h"
#include "gtkmm/grid.h"
#include "gtkmm/image.h"
#include "gtkmm/label.h"
#include <gtkmm.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#define MAX_WIDTH 100
#define MAX_HEIGHT 100

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 180

enum class PlayerAction {
  Previous,
  PlayPause,
  Next,
};

class Loki : public Gtk::Window {
public:
  Loki() {
    set_title("Loki");
    set_default_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    grid.set_margin_left(20);
    grid.set_margin_right(20);
    grid.set_margin_top(20);
    grid.set_margin_bottom(20);

    grid.set_row_spacing(10);
    grid.set_column_spacing(30);

    grid.attach(status, 0, 0);
    grid.attach(title, 1, 0);
    grid.attach(artist, 1, 1);
    grid.attach(position, 2, 0);
    grid.attach(image, 3, 0, 2, 2);

    previous.signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &Loki::on_button_click), PlayerAction::Previous));
    previous.set_label("Previous");
    grid.attach(previous, 0, 4);

    playPause.signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &Loki::on_button_click), PlayerAction::PlayPause));
    playPause.set_label("Play/Pause");
    grid.attach(playPause, 1, 4);

    next.signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &Loki::on_button_click), PlayerAction::Next));
    next.set_label("Next");
    grid.attach(next, 2, 4);

    add(grid);
    show_all_children();

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Loki::on_timeout),
                                   250);
  }

private:
  Gtk::Grid grid;

  Gtk::Button previous;
  Gtk::Button playPause;
  Gtk::Button next;

  void on_button_click(PlayerAction pa) {
    switch (pa) {
    case PlayerAction::Previous:
      exec("playerctl previous");
      break;
    case PlayerAction::PlayPause:
      exec("playerctl play-pause");
      status.set_text(exec("playerctl status"));
      break;
    case PlayerAction::Next:
      exec("playerctl next");
      break;
    }
  }

  Gtk::Label title;
  Gtk::Label artist;
  Gtk::Label position;
  Gtk::Label status;
  Gtk::Image image;

  bool on_timeout() {
    title.set_text(exec("playerctl metadata title"));
    artist.set_text(exec("playerctl metadata artist"));
    status.set_text(exec("playerctl status"));
    position.set_text(get_position());

    std::string img_uri = exec("playerctl metadata mpris:artUrl");
    if (!img_uri.empty()) {
      if (img_uri.compare(0, 7, "file://") == 0) {
        std::string img_path = img_uri.substr(7);
        load_image_async(img_path, MAX_WIDTH, MAX_HEIGHT);
      } else if (img_uri.compare(0, 7, "http://") == 0 ||
                 img_uri.compare(0, 8, "https://") == 0) {
        download_image_async(img_uri, MAX_WIDTH, MAX_HEIGHT);
      }
    }

    return true; // return true to keep the timeout active
  }

  void load_image_async(const std::string &path, int max_w, int max_h) {
    std::thread([this, path, max_w, max_h]() {
      try {
        auto pixbuf = Gdk::Pixbuf::create_from_file(path);
        auto scaled = scale_pixbuf_to_max(pixbuf, max_w, max_h);

        Glib::signal_idle().connect_once([this, scaled]() {
          image.set(scaled);
          if (!image.get_parent()) {
            grid.attach(image, 0, 1, 3, 1);
            show_all_children();
          }
        });
      } catch (const Glib::Error &err) {
        std::cerr << "Failed to load image: " << err.what() << std::endl;
      }
    }).detach();
  }

  void download_image_async(const std::string &url, int max_w, int max_h) {
    std::thread([this, url, max_w, max_h]() {
      std::string tmpfile = "/tmp/mpris_image.jpg";
      std::string cmd = "wget -q -O " + tmpfile + " \"" + url + "\"";
      int ret = system(cmd.c_str());

      if (ret == 0) {
        try {
          auto pixbuf = Gdk::Pixbuf::create_from_file(tmpfile);

          // Scale pixbuf to fit max dimensions
          auto scaled = scale_pixbuf_to_max(pixbuf, max_w, max_h);

          Glib::signal_idle().connect_once([this, scaled]() {
            image.set(scaled);
            if (!image.get_parent()) {
              grid.attach(image, 0, 1, 3, 1);
              show_all_children();
            }
          });
        } catch (const Glib::Error &err) {
          std::cerr << "Failed to load image from downloaded file: "
                    << err.what() << std::endl;
        }
      } else {
        std::cerr << "Failed to download image: wget returned " << ret
                  << std::endl;
      }
    }).detach();
  }

  Glib::RefPtr<Gdk::Pixbuf>
  scale_pixbuf_to_max(const Glib::RefPtr<Gdk::Pixbuf> &pixbuf, int max_width,
                      int max_height) {
    int width = pixbuf->get_width();
    int height = pixbuf->get_height();

    double scale = 1.0;

    if (width > max_width) {
      scale = std::min(scale, (double)max_width / width);
    }
    if (height > max_height) {
      scale = std::min(scale, (double)max_height / height);
    }

    if (scale < 1.0) {
      int new_width = static_cast<int>(width * scale);
      int new_height = static_cast<int>(height * scale);
      return pixbuf->scale_simple(new_width, new_height, Gdk::INTERP_BILINEAR);
    }

    return pixbuf; // no scaling needed
  }

  std::string get_position() {
    std::string pos_str = exec("playerctl position");
    if (pos_str.length() == 0)
      return "";
    double pos = std::stod(pos_str);

    std::string len_str = exec("playerctl metadata mpris:length");
    long long len_micro = std::stoll(len_str);
    double len_sec = len_micro / 1'000'000.0;

    // Convert to int seconds
    int pos_sec = static_cast<int>(pos);
    int len_sec_int = static_cast<int>(len_sec);

    int pos_min = pos_sec / 60;
    int pos_s = pos_sec % 60;

    int len_min = len_sec_int / 60;
    int len_s = len_sec_int % 60;

    std::ostringstream oss;
    oss << pos_min << ':' << std::setfill('0') << std::setw(2) << pos_s << " / "
        << len_min << ':' << std::setfill('0') << std::setw(2) << len_s;

    return oss.str();
  }

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
};

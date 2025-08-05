#include <gtkmm.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <libayatana-appindicator/app-indicator.h>

#include "../headers/utils.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 180

class Loki : public Gtk::Window {
public:
  Loki(Glib::RefPtr<Gtk::Application> app) : m_app(app) {
    set_title("Loki");
    set_default_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Create the Tree model:
    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);

    // Add the TreeView, inside a ScrolledWindow, with the button underneath:
    m_ScrolledWindow.add(m_TreeView);

    // Only show the scrollbars when they are necessary:
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    add(m_ScrolledWindow);

    // Fill the TreeView's model
    m_TreeView.append_column("Window Name", m_Columns.m_col_window_name);
    m_TreeView.append_column("Usage Time", m_Columns.m_col_usage_time);
    m_TreeView.append_column("Active Time", m_Columns.m_col_active_time);

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Loki::on_timeout),
                                   250);

    // Tray Icon
    setup_tray_icon();

    show_all_children();
    m_app->hold();
  }

protected:
  // Override the default close-button handler.
  bool on_delete_event(GdkEventAny *event) override {
    hide(); // Hide window instead of closing
    return true;
  }

private:
  void setup_tray_icon() {
    // Create a new AppIndicator
    indicator = app_indicator_new("loki-app", "emblem-synchronizing",
                                  APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

    // Create the menu
    Gtk::Menu *pMenu = Gtk::manage(new Gtk::Menu());

    // Create menu items
    Gtk::MenuItem *pShowHideItem =
        Gtk::manage(new Gtk::MenuItem("Show/Hide", true));
    pShowHideItem->signal_activate().connect(
        sigc::mem_fun(*this, &Loki::on_tray_show_hide));
    pMenu->append(*pShowHideItem);

    Gtk::MenuItem *pQuitItem = Gtk::manage(new Gtk::MenuItem("Quit", true));
    pQuitItem->signal_activate().connect(
        sigc::mem_fun(*this, &Loki::on_tray_quit));
    pMenu->append(*pQuitItem);

    pMenu->show_all();

    app_indicator_set_menu(indicator, GTK_MENU(pMenu->gobj()));
  }

  void on_tray_show_hide() {
    if (is_visible()) {
      hide();
    } else {
      show();
    }
  }

  void on_tray_quit() { m_app->release(); }
  // Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord {
  public:
    ModelColumns() {
      add(m_col_window_name);
      add(m_col_usage_time);
      add(m_col_active_time);
    }

    Gtk::TreeModelColumn<Glib::ustring> m_col_window_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_usage_time;
    Gtk::TreeModelColumn<Glib::ustring> m_col_active_time;
  };

  ModelColumns m_Columns;

  // Child widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;
  Gtk::TreeView m_TreeView;
  Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
  AppIndicator *indicator;
  Glib::RefPtr<Gtk::Application> m_app;

  bool on_timeout() {
    auto data = read_csv("/home/alice/.cache/camel-my-dreams/2025-08-05.csv");
    m_refTreeModel->clear();
    for (const auto &row_data : data) {
      Gtk::TreeModel::Row row = *(m_refTreeModel->append());
      row[m_Columns.m_col_window_name] = row_data[0];
      row[m_Columns.m_col_usage_time] = row_data[1];
      row[m_Columns.m_col_active_time] = row_data[2];
    }
    return true;
  }
};

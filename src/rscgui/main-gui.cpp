#include <QApplication>

#include <rscgui.hpp>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  rscui::RSCGui gui;

  gui.show();

  return app.exec();
}

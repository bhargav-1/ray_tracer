#include <fmt/base.h>    // for print
#include <version.h>     // for BUILD_VERSION_STRING, RELEASE_VERSION_STRING
#include <QApplication>  // for QApplication
#include <QPushButton>   // for QPushButton

auto main(int argc, char* argv[]) -> int {
  fmt::print("UI: Version: {}, Build: {}\n", RELEASE_VERSION_STRING, BUILD_VERSION_STRING);
  QApplication a(argc, argv);
  QPushButton button("Hello world!", nullptr);
  button.resize(200, 100);
  button.show();
  return QApplication::exec();
}

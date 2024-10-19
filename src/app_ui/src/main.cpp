#include <app/app.h>  // for AITD2App
#include <fmt/core.h>             // for print
#include <simpleble/Adapter.h>    // for Adapter
#include <version.h>              // for BUILD_VERSION_STRING, RELEASE_VERSI...
#include <QApplication>           // for QApplication
#include <QPushButton>            // for QPushButton
#include <exception>              // for exception
#include <iostream>               // for operator<<, endl, basic_ostream
#include <string>                 // for operator<<, char_traits
#include <vector>                 // for vector

auto Add(int x, int y) -> int { return x + y; }

auto main(int argc, char* argv[]) -> int {
  fmt::print("UI: Version: {}, Build: {}\n", RELEASE_VERSION_STRING, BUILD_VERSION_STRING);
  fmt::print("Testing Add: {}\n", App::Add(0, 1));
  QApplication a(argc, argv);
  QPushButton button("Hello world!", nullptr);
  button.resize(200, 100);
  button.show();

  try {
    if (!SimpleBLE::Adapter::bluetooth_enabled()) {
      std::cout << "Bluetooth is not enabled" << std::endl;
      return 1;
    }

    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty()) {
      std::cout << "No Bluetooth adapters found" << std::endl;
      return 1;
    }

    // Use the first adapter
    auto adapter = adapters[0];

    // Do something with the adapter
    std::cout << "Adapter identifier: " << adapter.identifier() << std::endl;
    std::cout << "Adapter address: " << adapter.address() << std::endl;
  } catch (const std::exception& e) {
    // Note to devs: This likely means dbus/bluez/bluetoothd failed to start or crashed
    std::cerr << "Failed to start/connect to bluetooth; Exception: " << e.what() << std::endl;
  }

  return QApplication::exec();
}

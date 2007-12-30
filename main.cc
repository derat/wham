#include "config.h"
#include "key-bindings.h"
#include "window-manager.h"
#include "x.h"

using namespace wham;

int main(int argc, char** argv) {
  XServer x_server;
  CHECK(x_server.Init());

  ref_ptr<Config> new_config(new Config);
  Config::Swap(new_config);

  KeyBindings bindings;
  CHECK(bindings.AddBinding("Ctrl+n", "create_anchor", NULL));
  x_server.RegisterKeyBindings(bindings);

  WindowManager window_manager;
  x_server.RunEventLoop(&window_manager);
  return 0;
}

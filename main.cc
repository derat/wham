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
  CHECK(bindings.AddBinding("Ctrl+n", "create_anchor", vector<string>(), NULL));
  CHECK(bindings.AddBinding("Ctrl+t", "exec",
                            vector<string>(1, "/usr/bin/urxvt"), NULL));
  CHECK(bindings.AddBinding("Ctrl+1", "switch_nth_window",
                            vector<string>(1, "0"), NULL));
  CHECK(bindings.AddBinding("Ctrl+2", "switch_nth_window",
                            vector<string>(1, "1"), NULL));
  CHECK(bindings.AddBinding("Ctrl+h", "switch_nearest_anchor",
                            vector<string>(1, "left"), NULL));
  CHECK(bindings.AddBinding("Ctrl+l", "switch_nearest_anchor",
                            vector<string>(1, "right"), NULL));
  CHECK(bindings.AddBinding("Ctrl+j", "switch_nearest_anchor",
                            vector<string>(1, "down"), NULL));
  CHECK(bindings.AddBinding("Ctrl+k", "switch_nearest_anchor",
                            vector<string>(1, "up"), NULL));
  x_server.RegisterKeyBindings(bindings);

  WindowManager window_manager;
  x_server.RunEventLoop(&window_manager);
  return 0;
}

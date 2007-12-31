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
  vector<string> args;
  CHECK(bindings.AddBinding("Ctrl+n", "create_anchor", args, NULL));
  args.push_back("/usr/bin/urxvt");
  CHECK(bindings.AddBinding("Ctrl+t", "exec", args, NULL));
  args.clear();
  args.push_back("0");
  CHECK(bindings.AddBinding("Ctrl+1", "switch_window", args, NULL));
  args.clear();
  args.push_back("1");
  CHECK(bindings.AddBinding("Ctrl+2", "switch_window", args, NULL));
  x_server.RegisterKeyBindings(bindings);

  WindowManager window_manager;
  x_server.RunEventLoop(&window_manager);
  return 0;
}

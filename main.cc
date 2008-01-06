#include "config.h"
#include "drawing-engine.h"
#include "key-bindings.h"
#include "window-manager.h"
#include "x.h"

using namespace wham;

int main(int argc, char** argv) {
  CHECK(XServer::Get()->Init());

  ref_ptr<DrawingEngine> drawing_engine(new DrawingEngine());
  DrawingEngine::Swap(drawing_engine);

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
  CHECK(bindings.AddBinding("Ctrl+g", "cycle_anchor_gravity",
                            vector<string>(1, "true"), NULL));
  CHECK(bindings.AddBinding("Ctrl+shift+g", "cycle_anchor_gravity",
                            vector<string>(1, "false"), NULL));
  XServer::Get()->RegisterKeyBindings(bindings);

  WindowManager window_manager;
  XServer::Get()->RunEventLoop(&window_manager);
  return 0;
}

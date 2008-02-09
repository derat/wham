#include "config.h"
#include "drawing-engine.h"
#include "key-bindings.h"
#include "window-manager.h"
#include "x-server.h"

using namespace wham;

int main(int argc, char** argv) {
  CHECK(XServer::Get()->Init());
  WindowManager window_manager;
  window_manager.SetupDefaultCrap();
  CHECK(window_manager.LoadConfig("config"));
  XServer::Get()->RunEventLoop(&window_manager);
  return 0;
}

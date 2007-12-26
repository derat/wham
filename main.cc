#include "window-manager.h"
#include "x.h"

using namespace wham;

int main(int argc, char** argv) {
  XServer x_server;
  CHECK(x_server.Init());
  WindowManager window_manager;
  x_server.RunEventLoop(&window_manager);
  return 0;
}

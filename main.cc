#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "drawing-engine.h"
#include "key-bindings.h"
#include "window-manager.h"
#include "x-server.h"

using namespace wham;

static const char* kUsage =
    "Usage: wham [options]\n"
    "\n"
    "Options:\n"
    "  -c FILE, --config=FILE   Config file to load\n"
    "  -h, --help               Display this message and exit\n";

int main(int argc, char** argv) {
  string config_file = "config";

  struct option long_opts[] = {
    { "config", true,  NULL, 'c' },
    { "help",   false, NULL, 'h' },
  };
  int opt = 0;
  while ((opt = getopt_long(argc, argv, "c:h", long_opts, NULL)) != -1) {
    switch (opt) {
      case 'c':
        config_file = string(optarg);
        break;
      case 'h':
        // fallthrough
      default:
        std::cerr << kUsage;
        exit(EXIT_FAILURE);
    }
  }

  CHECK(XServer::Get()->Init());
  WindowManager window_manager;
  window_manager.SetupDefaultCrap();
  CHECK(window_manager.LoadConfig(config_file));
  XServer::Get()->RunEventLoop(&window_manager);
  return 0;
}

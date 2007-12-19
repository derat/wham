#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"

using namespace std;
using namespace wham;

int main(int argc, char** argv) {
  Display* display = NULL;

  display = XOpenDisplay(NULL);
  if (!display) {
    LOG << "Can't open display";
    return 1;
  }

  ref_ptr<int> ptr(new int);
  ptr.reset(new int);

  return 0;
}

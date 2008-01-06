// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config.h"

using namespace std;

namespace wham {

ref_ptr<Config> Config::singleton_(new Config());

}  // namespace wham

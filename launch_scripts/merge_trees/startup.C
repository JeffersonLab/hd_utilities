#include "TTree.h"

int startup() {
  TTree::SetMaxTreeSize( 1000000000000LL ); // 1 TB
  return 0;
}

namespace {
  static int i = startup();
}

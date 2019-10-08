#include "processrun.hpp"

#include <iostream>
#include <cstdlib>
#include <type_traits>

#include <sys/stat.h>

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <root file>\n";
    return 1;
  }

  ProcessRun proc(
      "fcalBlockHits",
      argv[1]
  );
  proc.Run();

  return 0;
}

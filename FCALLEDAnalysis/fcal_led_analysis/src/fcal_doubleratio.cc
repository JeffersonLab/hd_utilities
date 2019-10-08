#include "doubleratio.hpp"

#include <iostream>
#include <cstdlib>
#include <type_traits>

#include <sys/stat.h>

int main(int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <root file> <output file>\n";
    return 1;
  }

  DoubleRatio dr(argv[1], argv[2]);
  dr.GetRatios();

  return 0;
}

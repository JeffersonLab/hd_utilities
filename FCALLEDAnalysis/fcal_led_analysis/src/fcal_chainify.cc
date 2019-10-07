#include "chainify.hpp"

#include <iostream>

int main(int argc, char **argv)
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0] << " <root file> <output file> <number of rings per chain>";
    std::cerr << " <min run> <max run>\n";
    return 1;
  }

  Chainify c(argv[1], argv[2], std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
  c.MakeChains();

  return 0;
}

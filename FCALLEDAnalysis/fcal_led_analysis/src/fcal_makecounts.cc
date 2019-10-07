#include "DFCALGeometry.hpp"
#include "ledutility.hpp"

#include <iostream>

int main(int argc, char **argv)
{
  DFCALGeometry dfcal{};
  Int_t counts[FCALLED::numrings];
  Int_t ncount = 0;

  for (unsigned int i=0; i < FCALLED::numchans; ++i) {
    auto dist = dfcal.positionOnFace((int)i).Mod();
    auto rin = FCALLED::GetRing(dist);
    ++counts[rin];
    if (dist >= FCALLED::innerdiam && dist <= FCALLED::outerdiam) {
      ++ncount;
    }
  }

  for (unsigned int i=0; i < FCALLED::numrings; ++i) {
    std::cout << "  " << counts[i] << "\n";
  }

  std::cout << "\n\n  " << ncount << "\n";

  return 0;
}

#ifndef LEDUTILITY_HPP_
#define LEDUTILITY_HPP_

#include <utility>
#include <TDataType.h>
#include "DFCALGeometry.hpp"

#include <string>

namespace FCALLED {
  typedef enum color_fake {
    COLOR_BLUE_LOW,
    COLOR_VIOLET_LOW,
    COLOR_GREEN,
    COLOR_VIOLET_HIGH,
    COLOR_BLUE_HIGH,
    COLOR_INVALID
  } COLOR;

  __attribute__((unused))
  static std::string BranchTitle(const char * l, unsigned int mid, const char * r) {
    std::string ret(l);
    ret += std::to_string(mid);
    ret += r;
    return ret;
  }

  static const unsigned int bigbuffer = 5000;
  static const unsigned int numchans = 2800;
  static const unsigned int numrings = 10;
  static const Float_t ring_range[numrings] = {
    5.8 * DFCALGeometry::k_cm,
    17.4 * DFCALGeometry::k_cm,
    29 * DFCALGeometry::k_cm,
    40.6 * DFCALGeometry::k_cm,
    52.2 * DFCALGeometry::k_cm,
    63.8 * DFCALGeometry::k_cm,
    75.4 * DFCALGeometry::k_cm,
    87.0 * DFCALGeometry::k_cm,
    98.6 * DFCALGeometry::k_cm,
    110.2 * DFCALGeometry::k_cm
  };
  static const Float_t noise_cutoff = 1000;
  static const Float_t epsil = 1e-8;
  static const Float_t innerdiam = 60 * DFCALGeometry::k_cm;
  static const Float_t outerdiam = 80 * DFCALGeometry::k_cm;
  static const Int_t ring_size[numrings] = {
    16,
    76,
    140,
    180,
    244,
    284,
    332,
    404,
    424,
    700
  };
  static const Int_t norm_size = 540;
  static const unsigned int numbranchpoints = 3;
  // If a ring has less branch points, pad the front with 0's
  static const Int_t branchpoints[numrings][numbranchpoints] = {
    {51130, // Ring 0
    56000,
    61063},
    {51130, // Ring 1
    56000,
    61063},
    {51130, // Ring 2
    56000,
    61063},
    {51130, // Ring 3
    56000,
    61063},
    {0, // Ring 4
    56000,
    61063},
    {0, // Ring 5
    56000,
    61063},
    {0, // Ring 6
    56000,
    61063},
    {0, // Ring 7
    56000,
    61063},
    {0, // Ring 8
    56000,
    61063},
    {0, // Ring 9
    56000,
    61063},
  };
  static const unsigned int numcolors = 5;
  static const unsigned int numcoloptions = 4; // Number of color settings
  static const Float_t col_range[numcoloptions][numcolors] = {
    {2500,
    4100,
    4800,
    7500,
    9000},

    {2400,
    4000,
    4700,
    7300,
    8900
    },

    {2500,
    4000,
    5000,
    7250,
    9250},

    {4000,
    6000,
    7000,
    9000,
    12000
    },
  };
  // Index into above array for each different normalization section on each ring
  // The 0 paddings can be whatever but must be present
  static const Int_t col_indices[numrings][numbranchpoints + 1] {
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
    {0, 0, 2, 3},
  };

  __attribute__((unused))
  static unsigned int GetNormSection(Int_t run, unsigned int ring)
  {
    size_t index = 0;
    // Find which branch point we are in
    //   (To get the correct color peaks for this run)
    if (run < FCALLED::branchpoints[ring][0]) {
      return 0;
    } else if (run >= FCALLED::branchpoints[ring][FCALLED::numbranchpoints - 1]) {
      return FCALLED::numbranchpoints;
    }
    for (unsigned int i=1; i < FCALLED::numbranchpoints; ++i) {
      if (run >= FCALLED::branchpoints[ring][i - 1] && run < FCALLED::branchpoints[ring][i]) {
        index = i;
      }
    }

    return index;
  }

  __attribute__((unused))
  static unsigned int GetColor(Float_t etot, Int_t run, unsigned int ring)
  {
    size_t index = GetNormSection(run, ring);

    // Find closest color
    unsigned int closest = static_cast<unsigned int>(-1);
    Float_t closest_appr = std::numeric_limits<Float_t>::max();
    for (unsigned int i=0; i < numcolors; ++i) {
      Float_t approach = TMath::Abs(col_range[col_indices[ring][index]][i] - etot);
      if (approach < closest_appr) {
        closest = i;
        closest_appr = approach;
      }
    }

    return closest;
  }

  __attribute__((unused))
  static unsigned int GetRing(Float_t rad)
  {
    unsigned int closest = static_cast<unsigned int>(-1);
    Float_t closest_appr = std::numeric_limits<Float_t>::max();

    for (unsigned int i=0; i < numrings; ++i) {
      Float_t approach = TMath::Abs(ring_range[i] - rad);
      if (approach < closest_appr) {
        closest = i;
      closest_appr = approach;
      }
    }

    return closest;
  }

  __attribute__((unused))
  static inline COLOR RangeColor(unsigned int n)
  {
    switch (n) {
      case 0: return COLOR_VIOLET_LOW;
      case 1: return COLOR_BLUE_LOW;
      case 2: return COLOR_GREEN;
      case 3: return COLOR_VIOLET_HIGH;
      case 4: return COLOR_BLUE_HIGH;
      default: return COLOR_INVALID;
    }
  }

  __attribute__((unused))
  static inline unsigned int ColorRange(COLOR c)
  {
    switch (c) {
      case COLOR_VIOLET_LOW: return 0;
      case COLOR_BLUE_LOW: return 1;
      case COLOR_GREEN: return 2;
      case COLOR_VIOLET_HIGH: return 3;
      case COLOR_BLUE_HIGH: return 4;
      case COLOR_INVALID:
      default: return static_cast<unsigned int>(-1);
    }
  }
}

#endif

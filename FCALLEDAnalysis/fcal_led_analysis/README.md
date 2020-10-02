---
title: GlueX FCAL Damage Analysis
author: Wyatt Campbell
date: 2019 July 25
---
# Table of Contents
1. [Analysis of LED data](#intro)

2. [Build System](#build)

3. [Data Structure](#data)  
  i. [processrun data format](#processout)  
  ii. [doubleratio data format](#doubleout)  
  iii. [chainify data format](#chainout)  

4. [Front Ends](#frontend)  
  i. [Front End Batch Files](#batch)

5. [Analysis Code](#analysis)

6. [ROOT Macros](#macros)  
  i. [Utility Header](#utility)

7. [Configuration](#config)  
  i. [processrun output](#datapath)  
  ii. [Tree Names](#treename)  
  iii. [Globals](#globals)  
  iv. [Color Settings](#settings)  
    1. [Branch Points](#bpoint)  
    2. [Color Settings](#colorset)  
    3. [Color Indices](#colorind)  

# <a name="intro"></a>Analysis of LED data
In order to monitor radiation damage on the FCAL, data from the
LED pulsars are used. The FCAL is split into ten concentric rings
around the beam hole, all 11.6 cm wide. Each pulsar color setting is
analyzed independently. The settings are:  
1. Violet LEDs, low voltage  
2. Blue LEDs, low voltage  
3. Green LEDs  
4. Violet LEDs, high voltage  
5. Blue LEDs, high voltage  


The analysis normalizes each run to a 20 cm wide ring on the FCAL detector
(at 60 cm - 80 cm radius, it is assumed no damage occurs in this area) in that run.
After normalization, the FCAL response to LEDs throughout time can be found. The
analysis produces "double ratios" of the normalized ring value in a run to the
normalized ring value of the initial run. The code allows for color settings
to be changed between runs on a per-ring basis, but if such a color setting
change occurs all rings with a run number lower that the change will be
compared to the run with the lowest run number *with that same color setting in
that ring*. Similarly, all rings in runs later than the change will be compared
to the ring in the run with the lowest run number for the new color setting
(i.e. they will be compared to the next run after the change).

Finally, series of runs are averaged. A sequence of n runs can be
averaged into a single value, called a chain. For example, 5 runs
(about half a day of data) can be combined into a single chain,
which retains the run number that is chronologically first out of
the 5 runs (e.g. Runs 8, 9, 11, 24, and 67 would combine into a chain
with run number 8). Chains never cross over color setting boundaries
(e.g. Runs 8, 9, 11, 24, and 67 would only make a chain with 8, 9, 11, and 24
if the color settings changed between 24 and 67).

# <a name="build"></a>Build System
This code uses a simple makefile for compilation. `root-config` is used
to link the code against ROOT. Note that the default compiler flags for
gcc contain very strict warnings which may need to be toned down by adjusting
the WFLAGS variable in the [makefile](./Makefile). All source files
are contained in [src](./src)

There are four types of files in the source tree:  
1. Front ends. These files end with `.cc`. They are compiled into executables with
  the same file name (sans the `.cc` extension) and linked against the analysis code
  object files.
2. ROOT macros. These files end with `.C` (note the upper case). These files
  are not compiled. Instead, they are loaded into ROOT as macro files with ROOT's
  Cling interpreter.
3. Analysis code. These files end with `.cpp`. They are compiled into object files. 
  If you want to use the code without the front ends, grab the `.cpp` files and
  the headers they need.
4. Header files. These end in `.h` or `.hpp`. The makefile
  automatically decides which source files requires which headers.
  
# <a name="output"></a>Data structure
Each stage of this analysis utilizes a different ROOT TTree structure for output.
Below the output formats of each stage is described.

## <a name="processout"></a>processruns output format
- The tree name is `fcalRings`.
- Each entry in the tree is a single run.
- `run` is an Int_t that contains the run number for this entry.
- `norm` is a Float_t that contains the normalization area average
  for that run.
- `nrms` is a Float_t that is the **standard error** of `norm`
- `ncount` is an Int_t that is the number of input data puts used to create the 
  normalization average.
- `rings` is an Int_t that contains the number of rings that were active during
  the run.
- `ringavg` is an array that contains `rings` Float_t's. Each array value is the average
  value of a ring over the run.
- `ringrms` is an array that contains `rings` Float_t's, each one the **standard error**
  of the corresponding `ringavg` value (*e.g.* `ringrms[1]` contains the standard
  error for `ringavg[1]`).
- `ringcnt` is an array that contains `rings` Int_t's, each one the number
  of input data points that went into the corresponding `ringavg` value
  (*e.g.* `ringcnt[2]` contains the number of input points for `ringavg[2]`).
- `ringid` is an array that contains `rings` Int_t's. Each array value is
  the ring id of the corresponding ringavg vale. Ring ids start at 0 for the inner
  most ring and go up by one for each consecutive ring (*i.e.* for 10 rings
  the inner most ring is id 0, the next ring from the center is ring 1, *etc.* up
  to ring 9 around the edge of the detector). `ringid[0]` tells you what ring
  `ringavg[0]` corresponds to, `ringid[1]` tells you what ring `ringavg[1]` corresponds
  to, *etc*.
  
## <a name="doubleout"></a>doubleratio output format
- The tree name is `fcalRingRatios`.
- Each entry in the tree is a single run.
- `run` is an Int_t that contains the run number for the entry.
- `rationum` is an Int_t that contains the number of single ratios
  (*i.e.* normalized ring values) in this entry.
- `ratio` is an arary of `rationum` Float_t's. Each array value is the normalized
  value for a ring.
- `ratiorms` is an array of `rationum` Float_t's, each one the **standard error**
  of the corresponding `ratio` value (*e.g.* `ratiorms[1]` contains the standard
  error for `ratio[1]`)
- `ratiocounts` is an array of `rationum` Int_t's, each one the number
  of input data points that went into the corresponding `ratio` value
  (*e.g.* `ratiocounts[1]` contains the number of input data points for
  `ratio[1]`)
- `ratioid` is an array of `rationum` Int_t's. Each array entry in `ratioid` is the
  ring id for the corresponding data in `ratio` (Ring ids start at 0 for the inner
  most ring and sequentially increase so that the outer most ring is ring id 9).
  That is, `ringid[1]` tells you which ring `ratio[1]` actually corresponds to.
- `drationum`, `dratio`, `dratiorms`, `dratiocounts`, and `dratioid` are exactly the
  same as their counterparts with out the d- prefix, but refer to double ratio data
  (i.e. normalized ring values divided by initial normalized ring value) instead
  of the simple single ratios.

## <a name="chainout"></a>chainify output format
- The tree name is `fcalRingChains`
- Each entry in the tree is a chain (*i.e.* the average of consecutive runs)
- `run` is an Int_t that contains the run number for the *first* run in the entry's chain.
- `ringnum` is an Int_t that contains the number of rings active in this chain.
- `ringavg` is an array of `ringnum` Float_t's. Each array value is
  the average double ratio for a ring in this chain.
- `ringrms` is an array of `ringnum` Float_t's. Each array value is the
  **standard error** of the corresponding `ringavg` value (*e.g.* `ringrms[1]`
  is the standard error of `ringavg[1]`)
- `ringcount` is an array of `ringnum` Int_t's. Each array value is the
  number of input data points that went into the corresponding double ratio
  value (*e.g.* `ringnum[1]` is the number of input data points that went into
  `ringavg[1]`)
- `ringid` is an array of `ringnum` Int_t's. Each array value is the ring id
  if the corresponding value in `ringavg`.
  (*e.g.* `ringid[1]` is the ring id for `ringavg[1]`). A ring id starts at 0 for
  the inner most ring and increases by one for each successive ring (*e.g.* the 
  outer most ring id for 10 rings is 9).

# <a name="frontend"></a>Front Ends
These programs take in GlueX LED data from an `fcalBlockHits` TTree and
finds trends over time. The analysis front-ends available are:

1. [`fcal_processruns`](./src/fcal_processruns.cc) which takes an input file of
  a GlueX run and breaks it into five sets of data, one for each LED color setting.
  The output data for each colors  contains the average for the normalization
  area in that run and averages for each of the FCAL detector rings over that run.
  - The only command line argument is the input ROOT file
  - By default, the output is set to be to files
    `data/data_{violet_low,blue_low,green,violet_high,blue_high}.root
  - This is designed to run in parallel on many inputs at once, so file
    mutexing is automatically done using POSIX's `lockf` function. The lock
    files also appear in "data/"

2. [`fcal_doubleratio`](./src/fcal_doubleratio.cc) which takes the output files
  of [`fcal_processruns`](./src/fcal_processruns.cc) and creates the double ratios
  from the run averages and normalization areas. 
  - Takes as arguments the input and output ROOT files. These paths may be the same.
  - This is designed to run on separate color files, so no file mutexing is done.
    Do not run two instances in parallel with the same output file
    (if the output is the same as the input, no other instances can read from the same
    file in parallel, either)

3. [`fcal_chainify`](./src/fcal_chainify.cc) which takes the output files of
  [`fcal_doubleratio`](./src/fcal_doubleratio.cc) and creates chains out of them.
  - Takes as arguments the input ROOT file, the output ROOT file (which may be the
  same as the input), and the number of runs to combine into each chain.
  - This is designed to run on separate color files, so no file mutexing is done.
    Do not run two instances in parallel with the same output file
    (if the output is the same as the input, no other instances can read from the same
    file in parallel, either)

## <a name="batch"></a>Front End Batch Files
The `.sh` files are examples on how to run the front ends from a script. These files
were used on the IU Bloomington Stanley compute cluster to analyze Fall 2018 data.
Use them as an example on how to run the code in parallel correctly.

# <a name="analysis"></a>Analysis Code
This is an explanation of all of the `.cpp` files in the source tree.

1. [`DFCALGeometry.cpp`](./src/DFCALGeometry.cpp) is a class from Matthew Shepherd.
  It is used to connect FCAL channel numbers into physical locations on the FCAL
  (i.e. channel id to physical row,column on the detector). It is also used to find
  the distance from the center of the detector.

2. [`processrun.cpp`](./src/processrun.cpp) is the class that takes input run data
  and transforms it into the ring and normalization area averages.
  - The input file is open for the lifetime of the class.
  - The data writing routine automatically  sorts the different LED colors into
    their own files.
  - the POSIX  file locking to allow for parallel access to the
    same output file is done internally.
  - The lock files remain after the code executes; scripts are responsible
    for deleting them (this is because there is no way for any one instance of the
    code to know if the other instances are done).
  - The constructor takes in the input ROOT file path and input ROOT tree name
    as parameters.
  - std::runtime_exception is thrown if the input or output files are inaccessible,
    or if the ROOT tree can not be found.

3. [`doubleratio.cpp`](./src/doubleratio.cpp) is the class that the ring and
  normalization averages and creates the double ratios.
  - The input file is open for the lifetime of the class.
  - The constructor takes in the input ROOT file path and the output ROOT file path

# <a name="macros"></a>ROOT Macros
ROOT macro functions are contained in [`rootscripts.C`](./src/rootscripts.C)
The functions are:

1. `DetectorGraph` - Given a run data file and radius range (by default
  0 cm to 117 cm, the whole graph), make a 2D color histogram of the
  average block responses over that run.
2. `RunExists` - Given a color, check to see if a run exists in the data set
  for that color
3. `MakeGraphs` - Given a data file and ring, make the graph of double ratios
  over a series of chains
4. `WriteGraphs` - Save graphs for all rings in a specified file. File must
  not already exist
5. `WriteMultigraphs` - Take in a ROOT file of graphs as written by `WriteGraphs`.
  Combine the high voltage blue, high voltage violet, and green LED data into
  a multigraph, looking at the specified ring.
6. `WriteAllMultigraphs` - Calls `WriteMultigraphs` for every ring.

**Note** These functions assume default output files from the analysis code.
If this is not the case, edit the `GetDataPath` function to get the
correct paths (and change the tree names if required).


## <a name="utility"></a>Utility Header
The file `ledutility` contains many globals and helper functions for
this code base. See [Configuration - Globals](#globals) for more information.

# <a name="config"></a>Configuration

## <a name="datapath"></a>processruns output
The per-color output files of `processruns` is stored in
one place. In the [`ProcessRun::DataPath(COLOR col)`](./src/processrun.cpp) function,
change the return values from the default paths to the
desired paths.  
- If you change the directory of the output, change
  the `mkdir` call in [`ProcessRun::WriteAvgs`](./src/processrun.cpp) to `mkdir("new director")`.
- If the new paths are not in a subdirectory, comment out the `mkdir` call.
- If the new paths are in nested subdirectories (*e.g* "data/leddata/file.root"),
  change the `mkdir` calls to iteratively make the directories
  (*e.g.*, `mkdir("data", 0777); mkdir("data/leddata", 0777);`)
  
**The lockfile paths must be in the same directory as the data paths**
If you change the data path directory, also change the lock path directories
in [`ProcessRun::LockPath(COLOR col)`](./src/processrun.cpp)

## <a name="treename"></a>Tree Names
The tree names for output can be modified for all classes. Furthermore, the
tree names for input can be modified easily for all classes except `processrun`.
In all cases, changing the names are as simple as changing a single value in the
class constructor.  
1. `processrun` - Change `otree("fcalRings")` to `otree("OUTPUT TREENAME")` in the
  constructor initializer list [process.cpp:20](./src/processrun.cpp)
2. `doubleratio` - Change `intreename("fcalRings")` to `intreename("INPUT TREENAME")`
  and `outtreename("fcalRingRatios")` to `outtreename("OUTPUT TREENAME")` in the
  constructor initializer list [doubleratio.cpp:13](./src/doubleratio.cpp)
3. `chainify` - Change `intreename("fcalRings")` to `intreename("INPUT TREENAME")`
  and `outtreename("fcalRingRatios")` to `outtreename("OUTPUT TREENAME")` in the
  constructor initializer list [chainify.cpp:18](./src/chainify.cpp)
  
**Note** If you change the output of one stage, remember to change the input of
the next stage as well.

## <a name="globals"></a>Globals
Many globals for the code are defined in [`ledutility.hpp`](./src/ledutility.hpp). 
All of these variables are under `namespace FCALLED`. These variables are static
so they can be used as compile-time constants that compilers will likely optimize.
In addition to globals, there are a few tiny static utility functions defined under the namespace.

The constants are:

1. `COLOR` - an enum of the different LED colors.
2. `BranchTitle` - Make the title of a branch with a variable array size
3. `bigbuffer` - The size of buffer needed to read in data from `fcalBlockHits`.
4. `numchans` - The number of detector channels on the FCAL
5. `numrings` - The number of rings to split the detector into
6. `ring_range` - The positions of the rings. If you wanted ring 0 to go from
  10 cm to 40 cm, you'd set ring_range[0] to 25 cm (the midpoint of the edges).
7. `noise_cutoff` - Minimum energy an event needs to be considered valid.
8. `epsil` - The epsilon used for comparing if variables are equal to 0.
9. `innerdiam` - The inner radius of the normalization area.
10. `outerdiam` - The outer radius of the normalization area.
11. `ring_size` - The number of blocks in each ring. The best way to generate this
  constant is to set the correct `numrings` and `ring_range`, then use a ROOT macro
  to go through all the FCAL channels, counting how many channels end up in each
  ring using `DFCALGeometry` and `FCALLED::GetRing`
12. `norm_size` - The number of blocks in the normalization area. The best way
  to generate this constant is with a ROOT macro as described above.
13. `numbranchpoints` - The number of places the color settings change
14. `branchpoints` - A 2D array of the run numbers of where the settings
  changes happens. The first index into this array is the ring id, the second
  index is the branch point number. Learn more in [Configuration - Color Settings](#settings)
15. `numcolors` - The number of colors present. If you change this, remember to actually
  change the `FCALLED::COLOR` enum to include the new colors
16. `numcoloptions` - The number of different color settings present in the data.
  Learn more in [Configuration - Color Settings](#settings)
17. `col_range` - Definitions for the different color settings. A 2D array
  where the first index is the color setting number and the second is the color.
  The numbers specified are the eTot peaks of each color. The order they appear in
  is determined by `FCALLED::ColorRange` and `FCALLED::RangeColor`.
  Learn more in [Configuration - Color Settings](#settings).
18. `col_indices` - a 2D array that specifies which color settings to use for what
  rings at what times. Learn more in [Configuration - Color Settings](#settings).
19. `GetNormSection` - Take in a run and a ring, return what color setting that ring
  is in for that run
20. `GetColor` - Given an event`s eTot, run number, and ring id, get the appropiate color
21. `GetRing` - Given a radius, get the appropiate ring
22. `RangeColor` - Given a color id, get the enum value
23. `ColorRange` - Given a color enum, get the color id

## <a name="settings"></a>Color Settings
**Note: it is important to remember to change all color globals if the you change the 
number of colors from 5**
The affected things are: `numcolors`, `COLOR`, `RangeColor`, `ColorRange`

In [`ledutility.hpp`](./src/ledutility.hpp), there is the ability to specify how
color settings change over time. The code will automatically handle any amount
of color setting changes once changed in that header. The color settings have the
ability to occur on a per-ring basis. Furthermore, it is possible to specify a
"color change" where the LED settings do not actually change. This is useful if some
other variable changes (like the PMT settings). The code will then just 
treat the change as a separation point where runs before and runs after
should not be compared with a double ratio. The components for this system are:

### <a name="bpoint"></a>Branch Points
A branch point is the point at which the color settings change.
The number of branch points is set with `FCALLED::numbranchpoints`. This must be set
to the maximum number of branch points any of the rings have. To change the branch points
per ring:

1. Change the nth inner ring to the chronological list of branch points for that
  ring. For example, setting the points for ring id 2 requires changing the array
  `branchpoints[2]`.

2. Pad the lists with 0's. If a ring has fewer branch points than `FCALLED::numbranchpoints`,
  pad the front of `branchpoints[ringid]` with 0's. For example, if ring id 2 had
  only one branch point at run 555, but `FCALLED::numbranchpoints` is 3, then
  `branchpoints[2]` would be: `{0, 0, 555}`.

### <a name="colorset"></a>Color Settings
`col_range` contains a list of possible color settings for
the data, in no particular order. Every entry into `col_range` is
an array of `FCALLED::numcolors` size. The entries into this subarray
are the eTot peaks for that color. For example, if in the eTot histogram
of a run color 3 peaks at 6000, then `subarray[3]` would be 6000.

Here's a fuller example. Say you have data with two colors and two distinct
color settings. color 0 in the first setting has an eTot peak of 5000, and
color 1 has a peak of 6000. In the second setting, color 0 has a peak of 7000
and color 1 has a peak of 8000. Then `color_range` would be:  
`{ {5000, 6000}, {7000, 8000}}`, where the order of the subarrays is arbitrary.

`numcoloptions` contains the total number of color options
(*i.e.* The outer index of `col_range`. In the last example, it would have been 2
since there were two distinct settings).

### <a name="colorind"></a>Color Indices
Color indices link a specific run period to a color setting.
`col_indices` contains this data. It is a 2D array, with the first
index being the ring id. The second dimension of the array must be
`numbranchpoints + 1` in size, to specify the regions of the data
before branch point 0, from branch point 0 to branch point 1,
*etc.* (including beyond the last branch point).

Set the value of `col_indcies[ring id]\[region]` to be the color setting index
you want that ring to use in that region between branch points.

For example, if you want ring id 0 to use the 3rd color setting in `col_range` 
in the region between branch points 3 and 4, you'd set `col_indicies\[0]\[3]`
to `3`.

For rings with zero padding at the front of the `branchpoints`, pad the front of
the `color_indices` entry with the first entry to be used.
For example, imagine ring id 3 had a branch point at run 555. The setting to be used
before this point was index 1, and the setting to be used after was index 0.
Furthermore, suppose that `numbranchpoints` was 3 so that the branch points of ring id 3
had to be padded to `{0, 0, 555}`. Then the `col_indices` for ring id 3
would be `{1, 1, 1, 0}`. The padded 0's were set to be the same index as the region
below run 555.

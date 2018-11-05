Various scripts for comparing different simulation versions.

Scripts to make PDFs of comparisons between hdgeant and hdgeant4 results in ROOT format.  1D histograms are overlain on one plot, 2D histograms are presented sequentially
- compare_hists.py - runs over entire ROOT file:
    Usage:  compare_hists.py [hdgeant_results.root] [hdgeant4_results.root]
- compare_hists_dirs.py - runs over a particular directory of a ROOT file, makes one PDF for each of its sub directories
    Use this e.g. for monitoring_hists, so that you don't have a thousand pages in one PDF...
    Note that this defaults to only plotting results under the /Independent directory
    Usage:  compare_hists_dirs.py [hdgeant_results.root] [hdgeant4_results.root] <optional base directory>
  
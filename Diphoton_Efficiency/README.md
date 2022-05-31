## Diphoton Efficiencies

The purpose of these studies was to investigate differences in the FCAL between the nominal clustering algorithm and the island algorithm, and to investigate the differences between using the center of gravity method for determining reconstructed shower positions and using a log-weighted average for shower positions.  To this end, two workflows were used: a single-photon gun, to evaluate the accuracy of the reconstruction, and a diphoton gun, to evaluate the efficiency of reconstructing two overlapping showers versus the distance between them.  As a reference, the CCAL was probed in the same way.

### Generators

##### DegFixed.py

A simple python script which produces a text file in genr8 format with two photons at specified angles and energies.  The run number, number of events to generate, output file name, and particle information can be adjusted.  The coded angles correspond to a small area in the FCAL insert region, for use with the FCAL1 or FCAL2 geometry, for testing the reconstruction of the FCAL1 blocks and FCAL2 insert with various clustering algorithms and shower reconstruction algorithms.  The same generator can be used to generate a single-photon gun by changing and commenting out the appropriate lines.

The output can be turned into an hddm file with the 'genr8\_2\_hddm' tool:

> genr8\_2\_hddm -P8.5 input.txt

##### DegFixedCCAL.py

This is the same as DegFixed.py, but the coded angles correspond to the active area of the CCAL when the photons are thrown from (0,0,800).  Commented-out values of theta correspond to the angles to be used when throwing the photons from the target center, (0,0,65).

When converting the output to an hddm file, the vertex position will have to be specified:

> genr8\_2\_hddm -P8.5 -V"0 0 800 800" input.txt

### Plugins

Plugins are run using 'hd\_root' over the output of a generator after said output has been run through HDGeant (and mcsmear).  The most straightforward way to do all of this is to use the 'MCWrapper' tool.  The hddm file from the generator, the output file location, the environment file, the desired hd\_root plugins (through a JANA configuration file), and more can be specified via an MCWrapper configuration file, and the desired run number and number of events to simulate and run over can be specified on the MCWrapper command line:

> $MCWRAPPER\_CENTRAL/gluex\_MC.py MCWrapper\_config\_file RunNumber/Range NumEvents \[Other Options\]

JANA configuration files are used to specify the plugins to be run over a dataset along with options for changing geometry variations, reconstruction parameters and algorithms, and many other controls.  Some JANA configuration file options useful for these studies are listed here:

> PLUGINS pluginname # Comma-separated list
> 
> JANA\_CALIB\_CONTEXT variation=mc\_JEF # Default is 'mc,' use 'mc\_JEF' for FCAL2 geometry and CCAL
> 
> DEFTAG:DFCALCluster Island # Use island clusterizer
> 
> FCAL:INSERT\_SHOWER\_WIDTH\_PARAMETER value # Island clusterizer b-parameter, current default is 0.69
> 
> FCAL:CHISQ\_MARGIN value # Island clusterizer fit Chi2 margin, current default is 5
> 
> FCAL:ENERGY\_SHARING\_CUTOFF value # Island clusterizer energy sharing cutoff in GeV, current default is 0.9
> 
> FCAL:MASS\_CUT value # Clusterizer mass cut in GeV, current default is 1E-6

##### FCAL\_diphoton\_gun\_hists

This plugin assumes two thrown photons incident on the FCAL.  In a double loop over the throwns and reconstructed FCAL showers, it projects the thrown photons to the depth of a reconstructed shower to check if each reconstructed shower aligns to a thrown.  If the thrown and reconstructed shower are close geometrically (2 cm difference in position at equal z) and energetically (difference of less than 350 MeV), the thrown is 'matched' to the shower.  If multiple showers match to a thrown, the closest in distance is taken to be the final match.  If both throwns are matched to different showers, the 'matchedSeparation' histogram is filled with the 'separation,' which is the distance between the two throws when both are projected to the FCAL face.  In all events, this separation is filled into the 'thrownSeparation' histogram, and the efficiency (effSeparation) is then the ratio of the matched to the thrown histograms.

A variant of the matched histogram is filled when both throwns are matched to showers _and_ only two FCAL showers exist (matchedSeparation\_2shower), and variants of both of these are filled after doing geometric matching using the log-weighted FCAL shower positions instead of the nominal center-of-gravity positions (\_logd).

##### CCAL\_diphoton\_gun\_hists

This plugin assumes two thrown photons incident on the CCAL.  If the generator file threw the photons from a custom vertex, that vertex should be specified in the .cc file of this plugin.  For most tests with this plugin, a vertex of (0,0,800) was used, and this is the current coded value.  The methodology and histograms produced for this plugin are identical to its FCAL counterpart, with a few small differences:

1) The CCAL showers do not report a z-position (depth) in the detector, so the throwns are projected to the CCAL face instead of the shower depth.
2) The histograms use the CCALShower 'x' and 'y' objects, and the '\_logd' variants use the 'x1' and 'y1' objects.  Rather than center-of-gravity vs. log-weighted positions, these are the shower fit positions and those positions projected to the CCAL face.

##### FCAL\_alt\_photon\_gun\_hists

This plugin assumes one thrown photon incident on the FCAL.  Here, the thrown is only matched energetically, not geometrically.  If there exists a reconstructed shower with about the correct energy, the various histograms are filled.  'deltaPosition' is the absolute distance between the thrown and matched shower with the thrown projected to the shower's depth, and 'deltax'/'deltay' are the thrown minus reconstructed x and y under the same conditions.  For 'deltax\_thrownx,' the y-axis is filled with the same value as the 'deltax' histogram, and the x-axis is the position of the projected thrown in the FCAL block (thrown x minus x-value at the center of the block all divided by the width of the block, giving a position in the block in percent).  'deltax\_matchedx' is similar, with the x-axis being the position of the reconstructed shower in the block.  These are useful for evaluating the bias of reconstructed shower positions toward the center of the blocks, especially using the center-of-gravity positions.  The same plots for y are also filled, and variants of all the above plots are filled using the log-weighted positions (\_logd).

Note: the FCALGeometry code that extracts the row/column of the block corresponding to an x or y position, and the code that uses the row and column to report the position of a block center, are currently hard-coded to use the FCAL1 geometry.  To get column and row numbers based on the FCAL2 geometry, the boolean flag in calls to the 'column' and 'row' functions should be changed to 'true.'

##### CCAL\_photon\_gun\_hists

This plugin assumes one thrown photon incident on the CCAL.  Again, tests with this plugin used a custom vertex of (0,0,800), so this must be altered if a different vertex is used.  The same plots are present here as in the 'FCAL\_alt\_photon\_gun\_hists' plugin, with the same few differences as the previous CCAL plugin:

1) The CCAL showers do not report a z-position (depth) in the detector, so the throwns are projected to the CCAL face instead of the shower depth.
2) The histograms use the CCALShower 'x' and 'y' objects, and the '\_logd' variants use the 'x1' and 'y1' objects.  Rather than center-of-gravity vs. log-weighted positions, these are the shower fit positions and those positions projected to the CCAL face.

##### FCAL\_alt\_diphoton\_gun\_hists

This plugin assumes two thrown photons incident on the FCAL.  It contains the same plots as the 'FCAL\_alt\_photon\_gun\_hists' plugin and uses only energetic matching.  Thus, the two thrown photons should have different energies so the plugin can match the throwns to the correct reconstructed showers.  As with the single-photon version of this plugin, the FCALGeometry calls to 'column' and 'row' are currently hard-coded to use the FCAL1 geometry, and to use the FCAL2 geometry, the boolean flag in those calls should be changed to 'true.'

### Scripts

ROOT scripts used to read in the ROOT files created by the above plugins and plot the created histograms.  In each of these, the input TFile(s) should be manually coded.  Currently, they are designed to accept one input file for each variation, so use 'hadd' to merge all ROOT files from an MCWrapper or similar execution.  Blocks of code can be commented out to plot only subsets of the coded plots, or they can be used as bases for custom pdfoutput scripts.

The legend entries, histogram names, histogram binning, PDF file page titles, etc. can be altered manually.  The only input argument for these scripts is a tag, which could be something like a date and/or descriptor, that will alter the output file name:

> root -q -b 'pdfoutput\_FCALPositions.C+("MAY13")'

##### pdfoutput\_DiphotonEfficiencies.C

This script takes ROOT files created using the FCAL\_diphoton\_gun\_hists and CCAL\_diphoton\_gun\_hists plugins.  In its current state, it takes input files for the FCAL1 and FCAL2 geometries using both the nominal clusterizer and the island clusterizer, and an input file for the CCAL.  It overlays the diphoton efficiency plots for each of these along with the '\_logd' variants.

##### pdfoutput\_DiphotonEfficiencies2.C

This is functionally the same as the above script, but instead taking eight input files (four variations each run with the nominal and island clusterizer).  It is set up to plot the center-of-gravity and log-weighted diphoton efficiencies for all four nominal clusterizer files, then for all four island clusterizer files.  This can be used to compare the effects of changing an energy threshold, an island algorithm parameter, or any other reconstruction variation.

##### pdfoutput\_FCALPositions.C

This script takes one ROOT file created using the FCAL\_alt\_photon\_gun\_hists or FCAL\_alt\_diphoton\_gun\_hists plugin.  It simply plots all the histograms in the ROOT file.

##### pdfoutput\_CCALPositions.C

This script takes one ROOT file created using the CCAL\_photon\_gun\_hists plugin.  It simply plots all the histograms in the ROOT file.

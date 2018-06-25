# ChiSq comparison tutorial for Analysis How To's
Author: Alex Barnes <br>
Created: 4/27/18

## Prerequisites
* sim-recon containing [$HALLD_HOME/src/program/Utilities/mergeTrees/mergeTrees.C](https://github.com/JeffersonLab/sim-recon/blob/master/src/programs/Utilities/mergeTrees/mergeTrees.C)
* gluex_root_analysis containing [DCutAction_ChiSqOrCL()](https://github.com/JeffersonLab/gluex_root_analysis/blob/master/libraries/DSelector/DCutActions.h)
* Output TTrees from the ReactionFilter plugin for your signal and background channels

## Running mergeTrees.C
Running the mergeTrees.C ROOT macro will create a new TTree file called **newtree.root** which is a clone of the signal TTree with two additional branches:
* ChiSq\_KinFit\_[background reaction name]
* NDF\_KinFit\_[background reaction name]

The reaction name come from the ReactionFilter's naming convention and is the first part of the TTree file name. For example, an analysis of K+K-p may want to remove Pi+Pi-p background. The additional branches would be called:
* ChiSq\_KinFit\_pippim
* NDF\_KinFit\_pippim

To run the macro, execute the following code. If the files are not in your current directory, include the full path.

`root -l -b -q '$HALLD_HOME/src/programs/Utilities/mergeTrees/mergeTrees.C+("SignalTree.root", "SignalTreeName", "BackgroundTree.root", "BackgroundTreeName")'`

If multiple backgrounds are being compared, the input signal file should be **newtree.root**.

`root -l -b -q '$HALLD_HOME/src/programs/Utilities/mergeTrees/mergeTrees.C+("newtree.root", "SignalTreeName", "BackgroundTree.root", "BackgroundTreeName")'`

## DSelector Analysis
Create a DSelector using **newtree.root** following the instructions found [here](https://halldweb.jlab.org/wiki/index.php/DSelector). Refer to DSelector_kpkm.h and DSelector_kpkm.C in this directory for using the cut action ChiSqOrCL.

The arguments for the cut action are:
* DParticleCombo* locParticleComboWrapper, the combo to be analyzed
* string locSecondaryReactionName, the name of the background reaction to be compared. This should match the tag of the branches that were created with 'mergeTrees.C'.
* bool locIsChiSq, true for using chisq values, false for using confidence level values
* TF1* locFunction, the function used to determine the cuts
* string locActionUniqueString, a unique identifier so that the same analysis action can be used more than once

The ChiSqOrCL cut action requires a TF1 as input which must be set up appropriately. In this example, a line was used with a default slope of 1.0 and y-intercept of 0.0. These parameters can be adjusted to find the optimal cut. Anything below the TF1 is kept. More complicated TF1 functions can be used if something other than a line is required.

This example DSelector made cuts prior to the ChiSq comparison cut, such as missing mass squared and PID delta t. This makes the effect easier to see but the cuts do not necessarily have to be in this order.

### Important Considerations
If there will be a cut on the ChiSq or confidence level, it recommended to be executed **after** the ChiSqOrCL comparison cut. The histograms provided by the comparison action can be useful in determining an appropriate chisq or confidence level cut.

If multiple comparisons are performed there **must** be a unique string as the last option of the cut action. This example used "Post_timing"

### Looking at the Results
In the output ROOT file there will be a directory called 'Cut_ChiSqOrCL'. If a unique string was supplied, the directory will be called 'Cut\_ChiSqOrCL\_[unique string]'. This is needed when using the same cut action multiple times.

Inside this directory are subdirectories containing histograms before and after the cut was applied. The "Before" directory shows the chisq and confidence level distributions for the signal and for the background in question. One particularly useful histogram is the 2D chisq comparison histogram which can help in determining the parameters of the TF1.

The "After" directory contains chisq and confidence level histograms after applying the cut. Both the remaining and cut signal chisq distributions are shown.

## Using TEnv for systematic studies
Rather than changing the TF1 parameters by hand, an easy way to test different cuts is to use ROOT's TEnv and provide the DSelector with a configuration file containing the cut parameters. An example configuration file is 'config_test_sig.cfg'. It contains a list of variables that have been defined in the DSelector with their corresponding values. When running batch jobs, each job can have a different config file to more efficiently run systematic studies.

To use this config file, run the DSelector as follows:
<br>
`$ root -l newtree.root`
<br>
`root [0] .x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C`
<br>
`root [1] kpkm_Tree->Process("DSelector_kpkm.C+", "config_test_sig.cfg")`

The config file in this tutorial overrides the initial slope of 1.0 with a new value of 0.5. This can be extended to any variable under study.

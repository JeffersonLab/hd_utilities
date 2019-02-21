This section of hd_utilities contains tools for analyzing photon efficiencies. Standard GlueX software setup including MCWrapper is assumed.

A list of contents:


CustomMCgen folder contains scripts for the generation of various MC samples including:
> Photon gun: with fixed theta(/energy), steps of energy(/theta) in different directories. (MCWrapper with queue submission)
** Run MakeSubmitEScan.py script to generate, see help/usage for arguments
** You must have compiled photon_gun_hists plugin (also included)! Plugin is run during MC generation. 
** Modify variable BASE_DIR to specify output directory (leaving empty will run in current directory)
** Add your personal copy of ccdb and rcdb to 
> Photon gun with embedded proton: 
** Protons according to some t-distribution roughly mocking up omega decays
** Procedure are arguments essentially the same on the front-end
** However, MCWrapper is not used, as the "generator" here is not currently supported
** Photon is given a phi value opposite proton. Theta and E are still specified elsewhere.
** Protons according to some t-distribution roughly mocking up omega decays, pi+ and pi- from omega phase space at fixed beam E
** Same as above, apart from addition of pions
> Photon gun with omega phase space modeling
** Generates a photon in a narrow range of E(/theta) according to omega phase space (at fixed beam E)
** Instead of fixing theta(/E), now generate according to omega phase space. Efficiency varies on energy AND amount of material in the way (which is theta-dependent). This should mock up data distributions better.
** If E is fixed, then theta now varies according to omega phase space, or vice versa. Additional minimum/maximum cuts can be applied (for instance, to require thrown photons also fall into FCAL)
NOTE THAT SAMPLES genPhotonGun_pipiP_scan, genPhotonGun_P_scan, and genPhotonGun_omegaweighted ARE WRITTEN FOR QSUB SUBMISSION SYSTEM! Other submission systems can be incorporated upon request to Jon Zarling at jzarling@jlab.org

photon_gun_hists plugin:
** Makes histograms of MC with a thrown photon
** If your control.in has TRAJECTORIES flag turned on (default for above samples) and events contains no other thrown particles, histograms indicating where the primary photon first interacts with something are generated
** FCAL showers currently supported. BCAL likely to be added.

DSelectors:
> DSelector_gammagun_proton.C: for a high-level analysis determination of efficiency using photon gun + proton generator
> DSelector_gammagun_pipiproton.C: for a high-level analysis determination of efficiency using photon gun + pi+pi- proton generator
> DSelector_FCAL_skimmer.C: for skimming useful events for omega->3pi analysis
> DSelector_omegaThrown.C:  for more detailed thrown information for omega->3pi MC samples
> (also, see Run_pipiP_selector.cxx for running over some trees. I used "+" and "-" characters in some of my tree names which wasn't a great idea)

Analyze output folder:
> For analyzing photon gun MC:
*** GetShowerLvlEffic.py   takes an output filename and hd_root.root files from photon gun MC and generates a low-level efficiency as a function of E(/theta), normalized to number of generated events
*** GetReactionLvlEffic.py gets high level analysis efficiency for photons embedded with proton, pi+pi- proton, or full omega cases. See documenation inside for use cases. Normalized to number of events without photon of study.
> For analyzing omega candidate events:
** FlattenMissShow.cxx flattens output from DSelector for omega candidate events
** MakeHists.cxx makes histograms for determining efficiencies with omega events
** FitHistsMakeEffic.cxx fits histograms output by MakeHists.cxx and determines an data-driven efficiency

The folder "HelpfulScripts" contains an example python script for running omega efficiency code with multiple sets of cut variations (e.g. spectator photon energy cut)
-- This is useful for demonstrating cut dependence of efficiency (or hopefully lack thereof).

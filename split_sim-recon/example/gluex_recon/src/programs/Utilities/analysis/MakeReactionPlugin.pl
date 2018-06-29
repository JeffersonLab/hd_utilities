#!/usr/bin/env perl

use Env;
use File::Copy;
Env::import();

$PluginName = "__nada__";
$ReactionName = "__nada__";
$ReactionFactoryTag = "__nada__";

if($#ARGV < 0)
{
	&Usage();
	exit;
}
elsif($#ARGV == 0)
{
	$PluginName = $ARGV[0];
	$ReactionName = $PluginName;
	$ReactionFactoryTag = $ReactionName;
}
elsif($#ARGV == 1)
{
	$PluginName = $ARGV[0];
	$ReactionName = $ARGV[1];
	$ReactionFactoryTag = $ReactionName;
}
else
{
	$PluginName = $ARGV[0];
	$ReactionName = $ARGV[1];
	$ReactionFactoryTag = $ARGV[2];
}

print "\n";
print "Generating files for plugin \"$PluginName\", with DReaction \"$ReactionName\" defined in DReaction_factory_${ReactionFactoryTag} \n";

# Create directory to hold source
mkdir($PluginName);

# Create C++ Header File for Factory Class
$dhfile = $fname = "DReaction_factory_${ReactionFactoryTag}.h";
open(FILE, ">$PluginName/$dhfile");
&PrintFileHeader();
&PrintFactoryClass();
close(FILE);
print " - $dhfile\n";

# Create C++ Implementation file for Factory Class
$ccfile = $fname = "DReaction_factory_${ReactionFactoryTag}.cc";
open(FILE, ">$PluginName/$ccfile");
&PrintFileHeader();
&PrintFactoryMethods();
close(FILE);
print " - $ccfile\n";

# Create SConscript
# Not really needed unless included in GlueX Build
#open(FILE, ">$PluginName/SConscript");
#&PrintSConscript();
#close(FILE);
#print " - SConscript\n";
#
# Create SConstruct
open(FILE, ">$PluginName/SConstruct");
&PrintSConstruct();
close(FILE);
print " - SConstruct\n";

# Copy SConstruct from build
#copy("${HALLD_RECON_HOME}/src/SBMS/SConstruct.plugin","${PluginName}/SConstruct") or die "Unable to copy SConstruct check HALLD_RECON_HOME is set correctly\r";
#print " - SConstruct\n";


# Create C++ Header File for Factory Generator Class
$dhfile = $fname = "DFactoryGenerator_${PluginName}.h";
open(FILE, ">$PluginName/$dhfile");
&PrintFileHeader();
&PrintFactoryGeneratorClass();
close(FILE);
print " - $dhfile\n";

# Create C++ Header File for Event Processor Class
$dhfile = $fname = "DEventProcessor_${PluginName}.h";
open(FILE, ">$PluginName/$dhfile");
&PrintFileHeader();
&PrintEventProcessorClass();
close(FILE);
print " - $dhfile\n";

# Create C++ Implementation file for Event Processor Class
$ccfile = $fname = "DEventProcessor_${PluginName}.cc";
open(FILE, ">$PluginName/$ccfile");
&PrintFileHeader();
&PrintEventProcessorMethods();
close(FILE);
print " - $ccfile\n";

###############
# PrintFileHeader
###############
sub PrintFileHeader()
{
	# print a few lines at the very top of the file
	$uname = `uname -nprs`;
	chomp($uname);
	print FILE "// \$Id\$\n";
	print FILE "//\n";
	print FILE "//    File: $fname\n";
	print FILE "// Created: ".`date`;
	print FILE "// Creator: ".$ENV{"USER"}." (on $uname)\n";
	print FILE "//\n";
}

###############
# PrintFactoryClass
###############
sub PrintFactoryClass()
{
	$content = "
\#ifndef _DReaction_factory_${ReactionFactoryTag}_
\#define _DReaction_factory_${ReactionFactoryTag}_

\#include <iostream>
\#include <iomanip>

\#include <JANA/JFactory.h>
\#include <ANALYSIS/DReaction.h>
\#include <ANALYSIS/DHistogramActions.h>
\#include <ANALYSIS/DCutActions.h>

using namespace std;
using namespace jana;

class DReaction_factory_${ReactionFactoryTag} : public jana::JFactory<DReaction>
{
	public:
		DReaction_factory_${ReactionFactoryTag}()
		{
			// This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
			SetFactoryFlag(PERSISTANT);
		}
		const char* Tag(void){return \"$ReactionFactoryTag\";}

	private:
		jerror_t evnt(JEventLoop* locEventLoop, uint64_t locEventNumber);
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks
};

\#endif // _DReaction_factory_${ReactionFactoryTag}_

";
	print FILE $content;
}

###############
# PrintFactoryMethods
###############
sub PrintFactoryMethods()
{
	$content = "

\#include \"DReaction_factory_${ReactionFactoryTag}.h\"

//------------------
// evnt
//------------------
jerror_t DReaction_factory_${ReactionFactoryTag}::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	// Make as many DReaction objects as desired
	DReactionStep* locReactionStep = NULL;
	DReaction* locReaction = NULL; //create with a unique name for each DReaction object. CANNOT (!) be \"Thrown\"

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software
	// DReaction factory: https://halldweb1.jlab.org/wiki/index.php/Analysis_DReaction

	/************************************************** ${ReactionName} Reaction Definition *************************************************/

	locReaction = new DReaction(\"${ReactionName}\");

	/* 
	//Required: DReactionSteps to specify the channel and decay chain you want to study
		//Particles are of type Particle_t, an enum defined in sim-recon/src/libraries/include/particleType.h

	//Example: g, p -> pi+, pi-, pi0, (p)
	//Inputs: Beam, target, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Gamma, Proton, {PiPlus, PiMinus, Pi0}, Proton);
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak

	//Example: pi0 -> g, g
	//Inputs: Decaying, non-missing final-state particles (vector), missing final state particle (none by default), bool inclusive_flag = false by default
	locReactionStep = new DReactionStep(Pi0, {Gamma, Gamma});
	//locReactionStep->Set_KinFitConstrainInitMassFlag(false); //default: true //ignored if p4 not fit or is beam //phi, omega not constrained regardless
	locReaction->Add_ReactionStep(locReactionStep);
	dReactionStepPool.push_back(locReactionStep); //register so will be deleted later: prevent memory leak
	*/

	/**************************************************** ${ReactionName} Control Settings ****************************************************/

	// Highly Recommended: Set EventStore skim query (use with \"eventstore\" source)
		// This will skip creating particle combos for events that aren't in the skims you list
		// Query should be comma-separated list of skims to boolean-AND together
	//locReaction->Set_EventStoreSkims(\"myskim1,myskim2,myskim3\"); //boolean-AND of skims

	// Recommended: Type of kinematic fit to perform (default is d_NoFit)
		//fit types are of type DKinFitType, an enum defined in sim-recon/src/libraries/ANALYSIS/DReaction.h
		//Options: d_NoFit (default), d_P4Fit, d_VertexFit, d_P4AndVertexFit
		//P4 fits automatically constrain decaying particle masses, unless they are manually disabled
	// locReaction->Set_KinFitType(d_P4AndVertexFit);

	// Highly Recommended: When generating particle combinations, reject all beam photons that match to a different RF bunch
	locReaction->Set_NumPlusMinusRFBunches(1); //1: 3 bunches, -1, 0, 1

	// Highly Recommended: Cut on number of extra \"good\" tracks. \"Good\" tracks are ones that survive the \"PreSelect\" (or user custom) factory.
		// Important: Keep cut large: Can have many ghost and accidental tracks that look \"good\"
	locReaction->Set_MaxExtraGoodTracks(4);

	// Highly Recommended: Enable ROOT TTree output for this DReaction
	// string is file name (must end in \".root\"!!): doen't need to be unique, feel free to change
	// locReaction->Enable_TTreeOutput(\"tree_${ReactionName}.root\", false); //true/false: do/don't save unused hypotheses

	/**************************************************** ${ReactionName} Analysis Actions ****************************************************/

	/*
	// Recommended: Analysis actions automatically performed by the DAnalysisResults factories to histogram useful quantities.
		//These actions are executed sequentially, and are executed on each surviving (non-cut) particle combination 
		//Pre-defined actions can be found in ANALYSIS/DHistogramActions_*.h and ANALYSIS/DCutActions.h
		//If a histogram action is repeated, it should be created with a unique name (string) to distinguish them

	// HISTOGRAM PID
	locReaction->Add_AnalysisAction(new DHistogramAction_PID(locReaction));

	// CUT PID
	// SYS_TOF, SYS_BCAL, SYS_FCAL, ...: DetectorSystem_t: Defined in libraries/include/GlueX.h
	// locReaction->Add_AnalysisAction(new DCutAction_EachPIDFOM(locReaction, 5.73303E-7));
	// locReaction->Add_AnalysisAction(new DCutAction_PIDDeltaT(locReaction, false, 1.0, Proton, SYS_TOF)); //cut at delta-t +/- 1.0 //false: measured data
	// locReaction->Add_AnalysisAction(new DCutAction_PIDTimingBeta(locReaction, 0.0, 0.9, Neutron, SYS_BCAL)); //min/max beta cut for neutrons
	// locReaction->Add_AnalysisAction(new DCutAction_NoPIDHit(locReaction, KPlus)); //for K+ candidates, cut tracks with no PID hit

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, \"Pi0_PreKinFit\"));
	locReaction->Add_AnalysisAction(new DHistogramAction_MissingMass(locReaction, false, 1000, 0.7, 1.2, \"PreKinFit\"));

	// KINEMATIC FIT
	// locReaction->Add_AnalysisAction(new DHistogramAction_KinFitResults(locReaction, 0.05)); //5% confidence level cut on pull histograms only
	// locReaction->Add_AnalysisAction(new DCutAction_KinFitFOM(locReaction, 0.0)); //0% confidence level cut //require kinematic fit converges

	// HISTOGRAM MASSES //false/true: measured/kinfit data
	//locReaction->Add_AnalysisAction(new DHistogramAction_InvariantMass(locReaction, Pi0, false, 600, 0.0, 0.3, \"Pi0_PostKinFit\"));
	//locReaction->Add_AnalysisAction(new DHistogramAction_MissingMass(locReaction, false, 1000, 0.7, 1.2, \"PostKinFit\"));

	// Kinematics
	locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, false)); //false: measured data
	// locReaction->Add_AnalysisAction(new DHistogramAction_ParticleComboKinematics(locReaction, true, \"KinFit\")); //true: kinematic-fit data
	locReaction->Add_AnalysisAction(new DHistogramAction_TrackVertexComparison(locReaction));
	*/

	_data.push_back(locReaction); //Register the DReaction with the factory

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DReaction_factory_${ReactionFactoryTag}::fini(void)
{
	for(size_t loc_i = 0; loc_i < dReactionStepPool.size(); ++loc_i)
		delete dReactionStepPool[loc_i]; //cleanup memory
	return NOERROR;
}

";
	print FILE $content;
}

###############
# PrintFactoryGeneratorClass
###############
sub PrintFactoryGeneratorClass()
{
	$content = "
\#ifndef _DFactoryGenerator_${PluginName}_
\#define _DFactoryGenerator_${PluginName}_

\#include <JANA/jerror.h>
\#include <JANA/JFactoryGenerator.h>

\#include \"DReaction_factory_${ReactionFactoryTag}.h\"

class DFactoryGenerator_${PluginName} : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return \"DFactoryGenerator_${PluginName}\";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DReaction_factory_${ReactionFactoryTag}());
			return NOERROR;
		}
};

\#endif // _DFactoryGenerator_${PluginName}_

";
	print FILE $content;
}

###############
# PrintEventProcessorClass
###############
sub PrintEventProcessorClass()
{
	$content = "
\#ifndef _DEventProcessor_${PluginName}_
\#define _DEventProcessor_${PluginName}_

\#include <JANA/JEventProcessor.h>
\#include <JANA/JApplication.h>

\#include <ANALYSIS/DEventWriterROOT.h>
\#include <HDDM/DEventWriterREST.h>
\#include <ANALYSIS/DHistogramActions.h>

\#include \"DFactoryGenerator_${PluginName}.h\"

using namespace jana;
using namespace std;

class DEventProcessor_${PluginName} : public jana::JEventProcessor
{
	public:
		const char* className(void){return \"DEventProcessor_${PluginName}\";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber);	///< Called every time a new run number is detected.
		jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);	///< Called every event.
		jerror_t erun(void);						///< Called every time run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		//For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

		//For making EventStore skims
		int Get_FileNumber(JEventLoop* locEventLoop) const;
		ofstream dEventStoreSkimStream;
};

\#endif // _DEventProcessor_${PluginName}_

";
	print FILE $content;
}

###############
# PrintEventProcessorMethods
###############
sub PrintEventProcessorMethods
{
	$content = "
\#include \"DEventProcessor_${PluginName}.h\"

// Routine used to create our DEventProcessor

extern \"C\"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->AddProcessor(new DEventProcessor_${PluginName}()); //register this plugin
		locApplication->AddFactoryGenerator(new DFactoryGenerator_${PluginName}()); //register the factory generator
	}
} // \"C\"

//------------------
// init
//------------------
jerror_t DEventProcessor_${PluginName}::init(void)
{
	// This is called once at program startup.

	/*
	//OPTIONAL: Create an EventStore skim.  
	string locSkimFileName = \"${ReactionName}.idxa\";
	dEventStoreSkimStream.open(locSkimFileName.c_str());
	dEventStoreSkimStream << \"IDXA\" << endl;
	*/

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_${PluginName}::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber)
{
	// This is called whenever the run number changes

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_${PluginName}::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// locEventLoop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	//
	// Here\'s an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// locEventLoop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

	/*********************************************************** REQUIRED ***********************************************************/

	//REQUIRED: To run an analysis, You MUST call one at least of the below code fragments. 
		//JANA is on-demand, so if you don't call one of these, then your analysis won't run. 

	/*
	//Recommended: Write surviving particle combinations (if any) to output ROOT TTree
		//If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations. 
		//The event writer gets the DAnalysisResults objects from JANA, performing the analysis. 
	// string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory
	const DEventWriterROOT* locEventWriterROOT = NULL;
	locEventLoop->GetSingle(locEventWriterROOT);
	locEventWriterROOT->Fill_DataTrees(locEventLoop, \"${ReactionFactoryTag}\");
	*/

	/*
	//Optional: Get the analysis results for all DReactions. 
		//Getting these objects triggers the analysis, if it wasn't performed already. 
		//These objects contain the DParticleCombo objects that survived the DAnalysisAction cuts that were added to the DReactions
	vector<const DAnalysisResults*> locAnalysisResultsVector;
	locEventLoop->Get(locAnalysisResultsVector);
	*/

	/************************************************** OPTIONAL: FURTHER ANALYSIS **************************************************/
	/*
	//Optional: Get all particle combinations for all DReactions. 
		//If kinematic fits were requested, these contain both the measured and kinematic-fit track parameters
		//No cuts from DAnalysisActions are placed on these combos
	vector<const DParticleCombo*> locParticleCombos;
	locEventLoop->Get(locParticleCombos);
	for(size_t loc_i = 0; loc_i < locParticleCombos.size(); ++loc_i)
	{
		const DParticleCombo* locParticleCombo = locParticleCombos[loc_i];
		if(locParticleCombo->Get_Reaction()->Get_ReactionName() != \"${ReactionName}\")
			continue; // particle combination was for a different reaction
		//perform further analysis steps here...
	}
	*/

	/*
	//Optional: Perform further cuts on the particle combos in the analysis results. 
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
		if(locAnalysisResults->Get_Reaction()->Get_ReactionName() != \"${ReactionName}\")
			continue; // analysis results were for a different reaction
		//get the DParticleCombo objects for this DReaction that survived all of the DAnalysisAction cuts
		deque<const DParticleCombo*> locPassedParticleCombos;
		locAnalysisResults->Get_PassedParticleCombos(locPassedParticleCombos);
		for(size_t loc_j = 0; loc_j < locPassedParticleCombos.size(); ++loc_j)
		{
			const DParticleCombo* locPassedParticleCombo = locPassedParticleCombos[loc_j];
			//perform further analysis steps here...
		}
	}
	*/

	/******************************************************** OPTIONAL: SKIMS *******************************************************/

	/*
	//Optional: Save event to output REST file. Use this to create physical skims.
	const DEventWriterREST* locEventWriterREST = NULL;
	locEventLoop->GetSingle(locEventWriterREST);
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
		if(locAnalysisResults->Get_Reaction()->Get_ReactionName() != \"${ReactionName}\")
			continue; // analysis results were for a different reaction

		//get the DParticleCombo objects for this DReaction that survived all of the DAnalysisAction cuts
		deque<const DParticleCombo*> locPassedParticleCombos;
		locAnalysisResults->Get_PassedParticleCombos(locPassedParticleCombos);

		if(!locPassedParticleCombos.empty())
			locEventWriterREST->Write_RESTEvent(locEventLoop, \"${ReactionName}\"); //string is part of output file name
	}
	*/

	/*
	//Optional: Create an EventStore skim. 

	// See whether this is MC data or real data
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);

	unsigned int locRunNumber = locEventLoop->GetJEvent().GetRunNumber();
	unsigned int locUniqueID = locMCThrowns.empty() ? 1 : Get_FileNumber(locEventLoop);

	// If a particle combo passed the cuts, save the event info in the output file
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
		if(locAnalysisResults->Get_Reaction()->Get_ReactionName() != \"${ReactionName}\")
			continue; // analysis results were for a different reaction
		if(locAnalysisResults->Get_NumPassedParticleCombos() == 0)
			continue; // no combos passed

		//MUST LOCK AROUND MODIFICATION OF MEMBER VARIABLES IN brun() or evnt().
		japp->WriteLock(\"${ReactionName}.idxa\"); //Lock is unique to this output file
		{
			dEventStoreSkimStream << locRunNumber << \" \" << locEventNumber << \" \" << locUniqueID << endl;
		}
		japp->Unlock(\"${ReactionName}.idxa\");
	}
	*/

	return NOERROR;
}

int DEventProcessor_${PluginName}::Get_FileNumber(JEventLoop* locEventLoop) const
{
	//Assume that the file name is in the format: *_X.ext, where:
		//X is the file number (a string of numbers of any length)
		//ext is the file extension (probably .evio or .hddm)

	//get the event source
	JEventSource* locEventSource = locEventLoop->GetJEvent().GetJEventSource();
	if(locEventSource == NULL)
		return -1;

	//get the source file name (strip the path)
	string locSourceFileName = locEventSource->GetSourceName();

	//find the last \"_\" & \".\" indices
	size_t locUnderscoreIndex = locSourceFileName.rfind(\"_\");
	size_t locDotIndex = locSourceFileName.rfind(\".\");
	if((locUnderscoreIndex == string::npos) || (locDotIndex == string::npos))
		return -1;

	size_t locNumberLength = locDotIndex - locUnderscoreIndex - 1;
	string locFileNumberString = locSourceFileName.substr(locUnderscoreIndex + 1, locNumberLength);

	int locFileNumber = -1;
	istringstream locFileNumberStream(locFileNumberString);
	locFileNumberStream >> locFileNumber;

	return locFileNumber;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_${PluginName}::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_${PluginName}::fini(void)
{
	// Called before program exit after event processing is finished.
	if(dEventStoreSkimStream.is_open())
		dEventStoreSkimStream.close();
	return NOERROR;
}

";
	print FILE $content;
}

$PluginName = "__nada__";
$ReactionName = "__nada__";
$ReactionFactoryTag = "__nada__";

###############
# PrintSConscript
###############
sub PrintSConscript()
{
	# Contents of default SConscript file for plugins
	print FILE "\n";
	print FILE "import sbms\n";
	print FILE "Import('*')\n";
	print FILE "env = env.Clone()\n";
	print FILE "sbms.AddDANA(env)\n";
	print FILE "sbms.AddROOT(env)\n";
	print FILE "sbms.plugin(env)\n";
	print FILE "\n";
}

###############
# Print SConstruct
###############

sub PrintSConstruct()
{
	# Contents of default SConstruct file for plugins
        $content = "

import os
import sys
import subprocess
import glob

# Get HALLD_RECON_HOME environment variable, verifying it is set
halld_home = os.getenv('HALLD_RECON_HOME')
if(halld_home == None):
        print 'HALLD_RECON_HOME environment variable not set!'
        exit(-1)

# Get HALLD_MY if it exists. Otherwise use HALLD_RECON_HOME
halld_my = os.getenv('HALLD_MY', halld_home)

# Get plugin name
pluginname = os.path.split(os.getcwd())[1]

# Add SBMS directory to PYTHONPATH
sbmsdir = \"%s/src/SBMS\" % halld_home
sys.path.append(sbmsdir)

import sbms

# Get command-line options
SHOWBUILD = ARGUMENTS.get('SHOWBUILD', 0)

# Get platform-specific name
osname = os.getenv('BMS_OSNAME', 'build')

# Get architecture name
arch = subprocess.Popen([\"uname\"], stdout=subprocess.PIPE).communicate()[0].strip()

# Setup initial environment
plugininstalldir = halld_my
gluexinstalldir = \"%s/%s\" %(halld_home, osname)
include = \"%s/include\" % (gluexinstalldir)
bin = \"%s/bin\" % (plugininstalldir)
lib = \"%s/lib\" % (plugininstalldir)
plugins = \"%s/%s/plugins\" % (plugininstalldir,osname)
env = Environment(        ENV = os.environ,  # Bring in full environement, including PATH
                      CPPPATH = [include],
                      LIBPATH = [\"%s/%s/lib\" %(halld_home, osname)],  # n.b. add HALLD_RECON_HOME here and prepend HALLD_MY below
                  variant_dir = \".%s\" % (osname))

# Only add HALLD_MY library search path if it already exists
# since we'll get a warning otherwise
if (os.path.exists(lib)): env.PrependUnique(lib)

# These are SBMS-specific variables (i.e. not default scons ones)
env.Replace(INSTALLDIR    = plugininstalldir,
                                OSNAME        = osname,
                                INCDIR        = include,
                                BINDIR        = bin,
                                LIBDIR        = lib,
                                PLUGINSDIR    = plugins,
                                ALL_SOURCES   = [],        # used so we can add generated sources
                                SHOWBUILD     = SHOWBUILD,
                 COMMAND_LINE_TARGETS = COMMAND_LINE_TARGETS)
# Use terse output unless otherwise specified
if SHOWBUILD==0:
        env.Replace(   CCCOMSTR       = \"Compiling  [\$SOURCE]\",
                                  CXXCOMSTR       = \"Compiling  [\$SOURCE]\",
                                  FORTRANPPCOMSTR = \"Compiling  [\$SOURCE]\",
                                  FORTRANCOMSTR   = \"Compiling  [\$SOURCE]\",
                                  SHCCCOMSTR      = \"Compiling  [\$SOURCE]\",
                                  SHCXXCOMSTR     = \"Compiling  [\$SOURCE]\",
                                  LINKCOMSTR      = \"Linking    [\$TARGET]\",
                                  SHLINKCOMSTR    = \"Linking    [\$TARGET]\",
                                  INSTALLSTR      = \"Installing [\$TARGET]\",
                                  ARCOMSTR        = \"Archiving  [\$TARGET]\",
                                  RANLIBCOMSTR    = \"Ranlib     [\$TARGET]\")

# Get compiler from environment variables (if set)
env.Replace( CXX = os.getenv('CXX', 'g++'),
             CC  = os.getenv('CC' , 'gcc'),
             FC  = os.getenv('FC' , 'gfortran') )

# Use C++11
env.PrependUnique(    CXXFLAGS = ['-std=c++11'])

# Add local directory, directories from HALLD_MY and HALLD_RECON_HOME to include search path
#env.PrependUnique(CPPPATH = ['#'])
env.PrependUnique(CPPPATH = ['%s/src' % halld_my, '%s/src/libraries' % halld_my, '%s/src/libraries/include' % halld_my])
env.PrependUnique(CPPPATH = ['%s/src' % halld_home, '%s/src/libraries' % halld_home, '%s/src/libraries/include' % halld_home])

# Turn on debug symbols and warnings
env.PrependUnique(      CFLAGS = ['-g', '-fPIC', '-Wall'])
env.PrependUnique(    CXXFLAGS = ['-g', '-fPIC', '-Wall'])
env.PrependUnique(FORTRANFLAGS = ['-g', '-fPIC', '-Wall'])

# Apply any platform/architecture specific settings
sbms.ApplyPlatformSpecificSettings(env, arch)
sbms.ApplyPlatformSpecificSettings(env, osname)


# Make plugin from source in this directory
sbms.AddDANA(env)
sbms.AddROOT(env)
sbms.plugin(env,\".%s/%s\" % (osname,pluginname))

# Make install target
env.Alias('install', env.Install(plugins, \".%s/%s.so\" % (osname,pluginname)))

";
        print FILE $content;
}

###############
# Usage
###############
sub Usage()
{
	print "\n";
	print "Usage:\n\n    MakeReactionPlugin plugin_name [reaction_name [reaction_factory_tag]] \n";
	print "\n    NOTE: The last two arguments are optional; they will be set to \"plugin_name\" and \n";
	print "    \"reaction_name\" (respectively) if not specified.\n";
	print "\n";
	print "Generate the C++ source and header files to implement a new analysis\n";
	print "plugin for use with the JANA framework. The plugin will implement\n";
	print "a DReaction factory (complete with factory generator).\n";
	print "\n";
	print "This will create a directory and generate a few files in it, including\n";
	print "a default SConstruct so that one can use to immediately compile the new plugin.\n";
	print "\n";

	print "\n";
	print "The C++ files generated will define classes based on JFactory,\n";
	print "JFactoryGenerator, and JEventProcessor. The \"plugin_name\" parameter is also\n";
	print "the name of the directory created (which can be changed).\n";
	print "\n";	
	print "\n";
}


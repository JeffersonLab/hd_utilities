#!/usr/bin/env python

# Tool for creating flux histograms from CCDB (ver 0.2)
# Author: Justin Stevens (jrsteven@jlab.org)

import os,sys
from ROOT import TFile,TGraph,TH1F,TF1
import rcdb
from optparse import OptionParser
from array import array
from datetime import datetime
import pprint
import math

import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet

def LoadCCDB():
    sqlite_connect_str = "mysql://ccdb_user@hallddb.jlab.org/ccdb"
    provider = ccdb.AlchemyProvider()                           # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)                        # use usual connection string to connect to database
    provider.authentication.current_user_name = "psflux_user"   # to have a name in logs

    return provider

def PSAcceptance(x, par):

    min = par[1]
    max = par[2]

    if x[0] > 2*min and x[0] < min + max:
        return par[0]*(1-2*min/x[0])
    elif x[0] >= min + max:
        return par[0]*(2*max/x[0] - 1)
    
    return 0.

def main():
    # Defaults
    RCDB_QUERY = "@is_production and @status_approved"
    RCDB_POLARIZATION = "" # AMO, PARA or PERP
    RCDB_POL_ANGLE = "" # 0, 45, 90, 135 (only 2017 and later)
    VARIATION = "default"
    CALIBTIME = datetime.now()

    BEGINRUN = 1
    ENDRUN = 100000000

    # Beam energy histogram defaults
    NBINS = 1000
    EMIN = 2.0
    EMAX = 12.0

    pp = pprint.PrettyPrinter(indent=4)

    # Define command line options
    parser = OptionParser(usage = "plot_flux_ccdb.py --begin-run beginRun --end-run endRun")
    parser.add_option("-b","--begin-run", dest="begin_run",
                     help="Starting run for output")
    parser.add_option("-e","--end-run", dest="end_run",
                     help="Ending run for output")
    parser.add_option("-p","--pol", dest="pol",
                     help="Polarization in RCDB")
    parser.add_option("-a","--angle", dest="angle",
                     help="Polarization angle in RCDB")
    parser.add_option("-n","--num-bins", dest="nbins",
		     help="Number of histogram bins")
    parser.add_option("-m","--energy-min", dest="emin",
		     help="Minimum energy for flux histogram")
    parser.add_option("-x","--energy-max", dest="emax",
                     help="Maximum energy for flux histogram")
    parser.add_option("-q","--rcdb-query", dest="rcdb_query",
                     help="RCDB query")
    parser.add_option("-t","--calib-time", dest="calib_time",
                     help="CCDB calibtime Y-M-D-h-min-s")

    (options, args) = parser.parse_args(sys.argv)

    if(not options.begin_run or not options.end_run):
        parser.print_help()
        sys.exit(0)

    if options.begin_run:
        BEGINRUN = int(options.begin_run)
    if options.end_run:
        ENDRUN = int(options.end_run)
    if options.pol:
        RCDB_POLARIZATION = options.pol
    if options.angle:
        RCDB_POL_ANGLE = options.angle
    if options.nbins:
	NBINS = int(options.nbins)
    if options.emin:
        EMIN = float(options.emin)
    if options.emax:
        EMAX = float(options.emax)
    if options.rcdb_query:
	RCDB_QUERY = options.rcdb_query
    if options.calib_time:
        try:
            CALIBTIME = datetime.strptime(options.calib_time, "%Y-%m-%d-%H-%M-%S")
        except:
            print "Calibration time format: Y-M-D-h-min-s"
            sys.exit(0)

    # Load CCDB
    print "CCDB calibtime = " + CALIBTIME.strftime("%Y-%m-%d-%H-%M-%S")
    ccdb_conn = LoadCCDB()

    # Load RCDB
    rcdb_conn = None
    try:
        rcdb_conn = rcdb.RCDBProvider("mysql://rcdb@hallddb.jlab.org/rcdb")
    except:
        e = sys.exc_info()[0]
        print "Could not connect to RCDB: " + str(e)
    
    # get run list
    runs = rcdb_conn.select_runs(RCDB_QUERY, BEGINRUN, ENDRUN)
    if len(runs) == 0:
        print("There are no runs matching the query \""+RCDB_QUERY +"\" between runs "+str(BEGINRUN)+" and "+str(ENDRUN))
        return

    photon_endpoint = array('f')
    tagm_tagged_flux = array('f')
    tagm_scaled_energy = array('f')
    tagh_tagged_flux = array('f')
    tagh_scaled_energy = array('f')

    htagged_fluxErr = TH1F("tagged_flux", "Tagged flux; Photon Beam Energy (GeV); Flux", NBINS, EMIN, EMAX)
    htagged_fluxErr.Sumw2()

    # Loop over runs
    for run in runs:
        if RCDB_POLARIZATION == "" and RCDB_POL_ANGLE != "":
            print "ERROR: polarization angle (option -a or --angle) specified, but polarization flag (option -p or --pol) was not. "
            print "Please rerun and specify a polarization flag (PARA or PERP) while running"
            return
			
        # select run conditions: AMO, PARA, and PERP and polarization angle
        if RCDB_POLARIZATION != "":
            conditions_by_name = run.get_conditions_by_name()
            conditions = run.get_conditions_by_name().keys()
            radiator_type = str(run.get_condition('radiator_type'))
            if "AMO" in RCDB_POLARIZATION:
                if 'RL' not in radiator_type and 'Al' not in radiator_type: 
                    continue
            elif run.get_condition('polarization_direction').value != RCDB_POLARIZATION:
                continue
            if RCDB_POL_ANGLE != "" and run.get_condition('polarization_angle').value != float(RCDB_POL_ANGLE):
                continue

	# temporary to exclude runs before status_approved flag is set for RunPeriod-2018-08
	if run.number == 51385 or run.number == 51404:
		continue

	print "==%d=="%run.number

	# Set livetime scale factor
	livetime_ratio = 0.0
	try:
		livetime_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/trig_live", run.number, VARIATION, CALIBTIME)
		livetime = livetime_assignment.constant_set.data_table
	        if float(livetime[3][1]) > 0.0: # check that livetimes are non-zero
        	       livetime_ratio = float(livetime[0][1])/float(livetime[3][1])
        	else: # if bad livetime assume ratio is 1
        	       livetime_ratio = 1.0
	except:
		livetime_ratio = 1.0 # default to unity if table doesn't exist
	
	# printout for livetimes different from unity
	#if livetime_ratio > 1.0 or livetime_ratio < 0.9:
	#	print livetime_ratio
	
	# Conversion factors for total flux
	converterThickness = run.get_condition('polarimeter_converter').value # 75 or 750 micron
	converterLength = 0 
	if converterThickness == "Be 75um": # default is 75 um
		converterLength = 75e-6
	elif converterThickness == "Be 750um":
    		converterLength = 750e-6
	elif run.number > 10633 and run.number < 10694: # no coverter in RCDB, but 75 um from logbook
		converterLength = 75e-6 
	else:
		print "Unknown converter thickness"
    	berilliumRL = 35.28e-2 # 35.28 cm
    	radiationLength = converterLength/berilliumRL;
    	scale = livetime_ratio * 1./((7/9.) * radiationLength);

	photon_endpoint_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/endpoint_energy", run.number, VARIATION, CALIBTIME)
	photon_endpoint = photon_endpoint_assignment.constant_set.data_table

	tagm_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagm/tagged", run.number, VARIATION, CALIBTIME)
	tagm_tagged_flux = tagm_tagged_flux_assignment.constant_set.data_table
	tagm_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/microscope/scaled_energy_range", run.number, VARIATION, CALIBTIME)
	tagm_scaled_energy = tagm_scaled_energy_assignment.constant_set.data_table

	tagh_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagh/tagged", run.number, VARIATION, CALIBTIME)
        tagh_tagged_flux = tagh_tagged_flux_assignment.constant_set.data_table
        tagh_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/hodoscope/scaled_energy_range", run.number, VARIATION, CALIBTIME)
        tagh_scaled_energy = tagh_scaled_energy_assignment.constant_set.data_table
	
        # fill tagm histogram
	for tagm_flux, tagm_scaled_energy in zip(tagm_tagged_flux, tagm_scaled_energy):
            tagm_energy = float(photon_endpoint[0][0])*(float(tagm_scaled_energy[1])+float(tagm_scaled_energy[2]))/2.
            bin_energy = htagged_fluxErr.FindBin(tagm_energy)
            previous_bincontent = htagged_fluxErr.GetBinContent(bin_energy)
            previous_binerror = math.sqrt(htagged_fluxErr.GetBinError(bin_energy)) # error^2 stored in histogram
            new_bincontent = previous_bincontent + float(tagm_flux[1]) * scale
            new_binerror = math.sqrt(previous_binerror*previous_binerror + float(tagm_flux[2])*float(tagm_flux[2])*scale*scale) 
	    htagged_fluxErr.SetBinContent(bin_energy, new_bincontent)
            htagged_fluxErr.SetBinError(bin_energy, new_binerror)
        
        # fill tagh histogram
	for tagh_flux, tagh_scaled_energy in zip(tagh_tagged_flux, tagh_scaled_energy):
            tagh_energy = float(photon_endpoint[0][0])*(float(tagh_scaled_energy[1])+float(tagh_scaled_energy[2]))/2.
	    bin_energy = htagged_fluxErr.FindBin(tagh_energy)
	    previous_bincontent = htagged_fluxErr.GetBinContent(bin_energy)
	    previous_binerror = math.sqrt(htagged_fluxErr.GetBinError(bin_energy)) # error^2 stored in histogram
	    new_bincontent = previous_bincontent + float(tagh_flux[1]) * scale
	    new_binerror = math.sqrt(previous_binerror*previous_binerror + float(tagh_flux[2])*float(tagh_flux[2])*scale*scale)
	    htagged_fluxErr.SetBinContent(bin_energy, new_bincontent)
	    htagged_fluxErr.SetBinError(bin_energy, new_binerror)

    # PS acceptance correction
    fPSAcceptance = TF1("PSAcceptance", PSAcceptance, 2.0, 12.0, 3);

    # Get parameters from CCDB 
    PS_accept_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/PS_accept", run.number, VARIATION, CALIBTIME)
    PS_accept = PS_accept_assignment.constant_set.data_table
    fPSAcceptance.SetParameters(float(PS_accept[0][0]), float(PS_accept[0][1]), float(PS_accept[0][2]));

    htagged_fluxErr.Divide(fPSAcceptance);

    # Initialize output file
    OUTPUT_FILENAME = "flux"
    if RCDB_POLARIZATION != "":
        OUTPUT_FILENAME += "_%s" % RCDB_POLARIZATION
    if RCDB_POL_ANGLE != "":
        OUTPUT_FILENAME += "_%s" % RCDB_POL_ANGLE
    OUTPUT_FILENAME += "_%d_%d.root" % (BEGINRUN, ENDRUN)
    
    fout = TFile(OUTPUT_FILENAME, "recreate")
    htagged_fluxErr.Write()
    fout.Close()
    

## main function 
if __name__ == "__main__":
    main()

#!/usr/bin/env python

# Tool for creating flux histograms from CCDB (ver 0.2)
# Author: Justin Stevens (jrsteven@jlab.org)

import os,sys
from ROOT import TFile,TGraph,TH1D,TF1,gRandom
import rcdb
from optparse import OptionParser
from array import array
from datetime import datetime
import pprint
import math
import MySQLdb

import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet

def LoadCCDB():
    sqlite_connect_str = "mysql://ccdb_user@hallddb.jlab.org/ccdb"
    provider = ccdb.AlchemyProvider()                           # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)                        # use usual connection string to connect to database
    provider.authentication.current_user_name = "psflux_user"   # to have a name in logs

    return provider

def loadCCDBContextList(runPeriod, restVer):
    dbhost = "hallddb.jlab.org"
    dbuser = 'datmon'
    dbpass = ''
    dbname = 'data_monitoring'

    conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
    curs=conn.cursor()    

    cmd = "SELECT revision,ccdb_context FROM version_info WHERE run_period=%s AND data_type='recon' AND revision<=%s ORDER BY revision DESC"
    curs.execute(cmd, [runPeriod, restVer])
    rows=curs.fetchall()
    return rows

def PSAcceptance(x, par):

    min = par[1]
    max = par[2]

    if x[0] > 2.*min and x[0] < min + max:
        return par[0]*(1-2.*min/x[0])
    elif x[0] >= min + max:
        return par[0]*(2.*max/x[0] - 1)
    
    return 0.

def main():

    # Defaults
    RCDB_QUERY = "@is_production and @status_approved"
    RCDB_POLARIZATION = "" # AMO, PARA or PERP
    RCDB_POL_ANGLE = "" # 0, 45, 90, 135 (only 2017 and later)
    VARIATION = "default"
    CALIBTIME = datetime.now()
    RESTVERSION = 999

    RCDB_QUERY_USER = RCDB_QUERY
    CALIBTIME_USER = CALIBTIME
    CALIBTIME_ENERGY = CALIBTIME

    BEGINRUN = 1
    ENDRUN = 100000000

    # Beam energy histogram defaults
    NBINS = 1000
    EMIN = 2.0
    EMAX = 12.0
    UNIFORM = False

    # Constants
    TARGETLENGTH = 29.5 # length in CM
    Navagadro = 6.02214e23 # atoms/mol
    units_g_mg = 1e-3
    units_cm2_b = 1e-24 # 1e-24 cm^2 = 1 barn

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
    parser.add_option("-u","--uniform", dest="uniform",
		     help="Uniform option")
    parser.add_option("-r","--rest-ver", dest="rest_ver",
                     help="REST version option")
    parser.add_option("-l","--target-length", dest="length",
                     help="Target length (in cm)")

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
	RCDB_QUERY_USER = options.rcdb_query
    if options.calib_time:
        try:
            CALIBTIME_USER = datetime.strptime(options.calib_time, "%Y-%m-%d-%H-%M-%S")
        except:
            print "Calibration time format: Y-M-D-h-min-s"
            sys.exit(0)
    if options.uniform:
	UNIFORM = True
    if options.rest_ver:
	RESTVERSION = options.rest_ver
    if options.length:
        TARGETLENGTH = float(options.length)

    # Run-dependent defaults for RCDB query
    if RCDB_QUERY != RCDB_QUERY_USER:
	RCDB_QUERY = RCDB_QUERY_USER
    else:
	if int(options.begin_run) >= 40000 and int(options.begin_run) < 60000: 
            # 2018-01 and 2018-11 run periods 
            RCDB_QUERY = "@is_2018production and @status_approved"
        if int(options.begin_run) >= 70000: 
            # 2019-11 run period
            RCDB_QUERY = "@is_dirc_production and @status_approved"
    
    print "RCDB quergy = " + RCDB_QUERY

    # REST production dependent CCDB calibtime
    if CALIBTIME != CALIBTIME_USER:
    	CALIBTIME = CALIBTIME_USER
	CALIBTIME_ENERGY = CALIBTIME_USER
    else: # get run period by run number
	runPeriod = "test"
	begin_run = int(options.begin_run)
	if begin_run < 20000: 
		runPeriod = "RunPeriod-2016-02"
	elif begin_run < 40000:
		runPeriod = "RunPeriod-2017-01"
        elif begin_run < 50000:
		runPeriod = "RunPeriod-2018-01"
        elif begin_run < 60000: 
		runPeriod = "RunPeriod-2018-08"
        elif begin_run < 70000:
		runPeriod = "RunPeriod-2019-01"
	elif begin_run < 80000:
                runPeriod = "RunPeriod-2019-11"
	contextList = loadCCDBContextList(runPeriod,RESTVERSION)
	RESTVERSION = contextList[0][0] # get REST version number from DB
	context = contextList[0][1] # get full JANA_CALIB_CONTEXT list from DB
	startCalibTime = context.find("calibtime")
	calibTimeString = context[startCalibTime+10:-1]
	CALIBTIME_ENERGY = datetime.strptime(calibTimeString , "%Y-%m-%d-%H-%M-%S")
	print "CCDB calibtime for energy to match REST ver%02d" % RESTVERSION + " = " + CALIBTIME_ENERGY.strftime("%Y-%m-%d-%H-%M-%S")

    # Load CCDB
    print "CCDB calibtime for flux = " + CALIBTIME.strftime("%Y-%m-%d-%H-%M-%S")
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

    photon_endpoint = array('d')
    tagm_tagged_flux = array('d')
    tagm_scaled_energy = array('d')
    tagh_tagged_flux = array('d')
    tagh_scaled_energy = array('d')

    if UNIFORM:
	htagged_flux = TH1D("tagged_flux_uniform", "Uniform tagged flux; Photon Beam Energy (GeV); Flux (# photons on target)", NBINS, EMIN, EMAX)
    htagged_fluxErr = TH1D("tagged_flux", "Tagged flux; Photon Beam Energy (GeV); Flux (# photons on target)", NBINS, EMIN, EMAX)
    htagm_fluxErr = TH1D("tagm_flux", "Tagged flux; TAGM Column; Flux", 102, 1, 103)
    htagh_fluxErr = TH1D("tagh_flux", "Tagged flux; TAGH Counter; Flux", 274, 1, 275)

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
	converterThicknessTable = run.get_condition('polarimeter_converter') # 75 or 750 micron
	converterThickness = ""
	if converterThicknessTable:
		converterThickness = converterThicknessTable.value		

	converterLength = 0 
	if converterThickness == "Be 75um": # default is 75 um
		converterLength = 75e-6
	elif converterThickness == "Be 750um":
    		converterLength = 750e-6
	elif run.number > 10633 and run.number < 10694: # no coverter in RCDB, but 75 um from logbook
		converterLength = 75e-6 
	else:
		print "Unknown converter thickness"
		sys.exit(0)

    	berilliumRL = 35.28e-2 # 35.28 cm
    	radiationLength = converterLength/berilliumRL;
    	scale = livetime_ratio * 1./((7/9.) * radiationLength);

        try:
            photon_endpoint_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/endpoint_energy", run.number, VARIATION, CALIBTIME_ENERGY)
            photon_endpoint = photon_endpoint_assignment.constant_set.data_table

            tagm_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagm/tagged", run.number, VARIATION, CALIBTIME)
            tagm_tagged_flux = tagm_tagged_flux_assignment.constant_set.data_table
            tagm_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/microscope/scaled_energy_range", run.number, VARIATION, CALIBTIME_ENERGY)
            tagm_scaled_energy_table = tagm_scaled_energy_assignment.constant_set.data_table
            
            tagh_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagh/tagged", run.number, VARIATION, CALIBTIME)
            tagh_tagged_flux = tagh_tagged_flux_assignment.constant_set.data_table
            tagh_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/hodoscope/scaled_energy_range", run.number, VARIATION, CALIBTIME_ENERGY)
            tagh_scaled_energy_table = tagh_scaled_energy_assignment.constant_set.data_table
        except:
            print "Missing flux for run number = %d, contact jrsteven@jlab.org" % run.number 
            sys.exit(0)

            
        # PS acceptance correction
        fPSAcceptance = TF1("PSAcceptance", PSAcceptance, 2.0, 12.0, 3);
        
        # Get parameters from CCDB 
        PS_accept_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/PS_accept", run.number, VARIATION, CALIBTIME)
        PS_accept = PS_accept_assignment.constant_set.data_table
        fPSAcceptance.SetParameters(float(PS_accept[0][0]), float(PS_accept[0][1]), float(PS_accept[0][2]));

        # Get corrections to tagger calib for run > 60000 (PrimEx,GlueX-II)
        photon_endpoint_calib = array('d')
        calibrated_endpoint = False
        try:
            photon_endpoint_calib_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/hodoscope/endpoint_calib", run.number, VARIATION, CALIBTIME_ENERGY)
            photon_endpoint_calib = photon_endpoint_calib_assignment.constant_set.data_table
            photon_endpoint_delta_E = float(photon_endpoint[0][0]) - float(photon_endpoint_calib[0][0])
            calibrated_endpoint = True
        except:
            if run.number > 60000:
                print "Missing endpoint calibration for run "+run.number
                sys.exit(0)

        # fill tagm histogram
	for tagm_flux, tagm_scaled_energy in zip(tagm_tagged_flux, tagm_scaled_energy_table):
	    tagm_energy = float(photon_endpoint[0][0])*(float(tagm_scaled_energy[1])+float(tagm_scaled_energy[2]))/2.
            if calibrated_endpoint:
                tagm_energy = float(photon_endpoint_calib[0][0])*(float(tagm_scaled_energy[1])+float(tagm_scaled_energy[2]))/2. + photon_endpoint_delta_E

            psAccept = fPSAcceptance.Eval(tagm_energy)
            if psAccept <= 0.0:
                continue

	    if UNIFORM:
	            tagm_energy_low = float(photon_endpoint[0][0])*(float(tagm_scaled_energy[1]))
		    tagm_energy_high = float(photon_endpoint[0][0])*(float(tagm_scaled_energy[2]))
                    if calibrated_endpoint:
                        tagm_energy_low = float(photon_endpoint_calib[0][0])*(float(tagm_scaled_energy[1])) + photon_endpoint_delta_E
                        tagm_energy_high = float(photon_endpoint_calib[0][0])*(float(tagm_scaled_energy[2])) + photon_endpoint_delta_E
	
                    flux = float(tagm_flux[1])
		    i = 0
		    while i <= flux:
			energy = tagm_energy_low + gRandom.Uniform(tagm_energy_high-tagm_energy_low)
		    	htagged_flux.Fill(energy,scale/fPSAcceptance.Eval(energy))
			i += 1

            bin_energy = htagged_fluxErr.FindBin(tagm_energy)
            previous_bincontent = htagged_fluxErr.GetBinContent(bin_energy)
            previous_binerror = htagged_fluxErr.GetBinError(bin_energy)

            current_bincontent = float(tagm_flux[1]) * scale / psAccept
            current_binerror = float(tagm_flux[2]) * scale / psAccept
            new_bincontent = previous_bincontent + current_bincontent
            new_binerror = math.sqrt(previous_binerror*previous_binerror + current_binerror*current_binerror) 
	    htagged_fluxErr.SetBinContent(bin_energy, new_bincontent)
            htagged_fluxErr.SetBinError(bin_energy, new_binerror)
            htagm_fluxErr.Fill(int(tagm_flux[0]), current_bincontent)
        
        # fill tagh histogram
	previous_energy_scaled_low = 999. # keep track of low energy bin boundry to avoid overlaps
	for tagh_flux, tagh_scaled_energy in zip(tagh_tagged_flux, tagh_scaled_energy_table):
            tagh_energy = float(photon_endpoint[0][0])*(float(tagh_scaled_energy[1])+float(tagh_scaled_energy[2]))/2.
            if calibrated_endpoint:
                tagh_energy = float(photon_endpoint_calib[0][0])*(float(tagh_scaled_energy[1])+float(tagh_scaled_energy[2]))/2. + photon_endpoint_delta_E

            psAccept = fPSAcceptance.Eval(tagh_energy)
            if psAccept <= 0.0:
                continue

	    if UNIFORM:
		    tagh_energy_low = float(photon_endpoint[0][0])*(float(tagh_scaled_energy[1]))
 	    	    tagh_energy_high = float(photon_endpoint[0][0])*(float(tagh_scaled_energy[2]))
                    if calibrated_endpoint:
                        tagh_energy_low = float(photon_endpoint_calib[0][0])*(float(tagh_scaled_energy[1])) + photon_endpoint_delta_E
                        tagh_energy_high = float(photon_endpoint_calib[0][0])*(float(tagh_scaled_energy[2])) + photon_endpoint_delta_E

		    if previous_energy_scaled_low < tagh_energy_high:
			tagh_energy_high = previous_energy_scaled_low

	            flux = float(tagh_flux[1])
	            i = 0
	            while i <= flux:
	                energy = tagh_energy_low + gRandom.Uniform(tagh_energy_high-tagh_energy_low)
	                htagged_flux.Fill(energy,scale/fPSAcceptance.Eval(energy))
	                i += 1

		    previous_energy_scaled_low = tagh_energy_low

	    bin_energy = htagged_fluxErr.FindBin(tagh_energy)
	    previous_bincontent = htagged_fluxErr.GetBinContent(bin_energy)
	    previous_binerror = htagged_fluxErr.GetBinError(bin_energy)

            current_bincontent = float(tagh_flux[1]) * scale / psAccept
            current_binerror = float(tagh_flux[2]) * scale / psAccept
	    new_bincontent = previous_bincontent + current_bincontent
	    new_binerror = math.sqrt(previous_binerror*previous_binerror + current_binerror*current_binerror)
	    htagged_fluxErr.SetBinContent(bin_energy, new_bincontent)
	    htagged_fluxErr.SetBinError(bin_energy, new_binerror)
            htagh_fluxErr.Fill(int(tagh_flux[0]), current_bincontent)

    # Get density factor from CCDB
    density = 0.0
    density_err = 0.0
    try:
        density_assignment = ccdb_conn.get_assignment("/TARGET/density", run.number, VARIATION, CALIBTIME)
        density_table = density_assignment.constant_set.data_table
        if float(density_table[0][0]) > 0.0: # check that livetimes are non-zero
            density = float(density_table[0][0])
            density_err = float(density_table[0][1])
    except:
        print("couldn't find density for run %s" % RUN)
        exit

    print("Target length = %0.2f cm" % TARGETLENGTH)
    print("Target density = %0.2f +/- %0.2f mg/cm^3" % (density, density_err))

    # Compute target length to compute luminosity
    targetScatteringCenters = density * TARGETLENGTH * Navagadro * units_cm2_b * units_g_mg
    targetScatteringCenters_err = targetScatteringCenters * density_err/density
    print("Target scattering center = %0.3f +/- %0.3f b^-1" % (targetScatteringCenters,targetScatteringCenters_err))
    htagged_lumiErr = htagged_fluxErr.Clone("tagged_lumi")
    htagged_lumiErr.Reset()
    htagged_lumiErr.SetTitle("Tagged luminosity (pb^{-1}); Photon Beam Energy (GeV); Luminosity (pb^{-1})")
    for i in range(1,htagged_fluxErr.GetNbinsX()):
        lumi = htagged_fluxErr.GetBinContent(i) * targetScatteringCenters / 1e12
        if htagged_fluxErr.GetBinContent(i) <= 0.0:
            continue
        fluxErr = htagged_fluxErr.GetBinError(i)/htagged_fluxErr.GetBinContent(i)
        targetErr = targetScatteringCenters_err/targetScatteringCenters
        lumiErr = lumi * math.sqrt(fluxErr*fluxErr + targetErr*targetErr)
        htagged_lumiErr.SetBinContent(i,lumi)
        htagged_lumiErr.SetBinError(i,lumiErr)

    # Initialize output file
    OUTPUT_FILENAME = "flux"
    if RCDB_POLARIZATION != "":
        OUTPUT_FILENAME += "_%s" % RCDB_POLARIZATION
    if RCDB_POL_ANGLE != "":
        OUTPUT_FILENAME += "_%s" % RCDB_POL_ANGLE
    OUTPUT_FILENAME += "_%d_%d.root" % (BEGINRUN, ENDRUN)
    
    fout = TFile(OUTPUT_FILENAME, "recreate")
    if UNIFORM:
	htagged_flux.Write()
    htagged_fluxErr.Write()
    htagged_lumiErr.Write()
    htagm_fluxErr.Write()
    htagh_fluxErr.Write()
    fout.Close()
    

## main function 
if __name__ == "__main__":
    main()

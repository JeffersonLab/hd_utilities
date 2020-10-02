#!/usr/bin/env python

# Tool for creating flux histograms from CCDB (ver 0.2)
# Modified version of the flux originally used by GlueX

import os,sys
import rcdb
from ROOT import TFile,TGraph,TH1F,TF1,gRandom, FILE
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
#    sqlite_connect_str = "sqlite:////home/somov/ccdb.sqlite"
    provider = ccdb.AlchemyProvider()                           # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)                        # use usual connection string to connect to database
    provider.authentication.current_user_name = "somov"   # to have a name in logs

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

    if x[0] > 2*min and x[0] < min + max:
        return par[0]*(1-2*min/x[0])
    elif x[0] >= min + max:
        return par[0]*(2*max/x[0] - 1)
    
    return 0.

def main():
    
    VARIATION   =  "default"

    conv_length   =  750e-6
    Be_rl         =  35.28e-2      
    conv_rl       =  conv_length/Be_rl;   # 2.1259 10-3   Used in the PS acceptance determination
        
    ps_scale = 1./((7/9.) * conv_rl)


    parser = OptionParser(usage = "plot_flux_ccdb.py --begin-run beginRun --end-run endRun")

    parser.add_option("-b","--begin-run", dest="begin_run",
                      help="Starting run for output")

    parser.add_option("-e","--end-run", dest="end_run",
                     help="Ending run for output")
    
    (options, args) = parser.parse_args(sys.argv)

    FIRST_RUN = int(options.begin_run)

    run_number = FIRST_RUN

    OUTPUT_FILE_TAGH = "%d_tagh_ps_acc_cor.txt" %(FIRST_RUN)
    OUTPUT_FILE_TAGM = "%d_tagm_ps_acc_cor.txt" %(FIRST_RUN)
    OUTPUT_FILE_ROOT = "%d_flux.root" %(FIRST_RUN)


    ccdb_conn = LoadCCDB()
        

    htagh_flux      =  TH1F("TAGH Tagged Flux","TAGH Tagged Flux",300,0.5,299.5)
    htagh_flux_cor  =  TH1F("TAGH Tagged Flux Cor","TAGH Tagged Flux Cor",300,0.5,299.5)

    htagm_flux      =  TH1F("TAGM Tagged Flux","TAGM Tagged Flux",120,0.5,199.5)
    htagm_flux_cor  =  TH1F("TAGM Tagged Flux Cor","TAGM Tagged Flux Cor",120,0.5,199.5)


    photon_endpoint_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/endpoint_energy",
                                                          run_number, VARIATION)
    
    photon_endpoint = photon_endpoint_assignment.constant_set.data_table
    
    tagh_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagh_tagged", 
                                                           run_number, VARIATION)
    
    tagh_tagged_flux = tagh_tagged_flux_assignment.constant_set.data_table
    
    tagm_tagged_flux_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/tagm_tagged", 
                                                           run_number, VARIATION)
    
    tagm_tagged_flux = tagm_tagged_flux_assignment.constant_set.data_table


    tagh_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/hodoscope/scaled_energy_range",run_number, VARIATION)

    tagh_scaled_energy = tagh_scaled_energy_assignment.constant_set.data_table

    tagm_scaled_energy_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/microscope/scaled_energy_range",run_number, VARIATION)

    tagm_scaled_energy = tagm_scaled_energy_assignment.constant_set.data_table

    # PS acceptance correction
    fPSAcceptance = TF1("PSAcceptance", PSAcceptance, 2.0, 12.0, 3);

    # Get parameters from CCDB 
    PS_accept_assignment = ccdb_conn.get_assignment("/PHOTON_BEAM/pair_spectrometer/lumi/PS_accept", run_number, VARIATION)
    
    PS_accept = PS_accept_assignment.constant_set.data_table
    

    fPSAcceptance.SetParameters(float(PS_accept[0][0]), float(PS_accept[0][1]), float(PS_accept[0][2]))
    
        

    data_file_tagh = open(OUTPUT_FILE_TAGH,"w+");

    for ii in range(274):
            
        htagh_flux.SetBinContent(int(tagh_tagged_flux[ii][0]),float(tagh_tagged_flux[ii][1]))
        htagh_flux.SetBinError(int(tagh_tagged_flux[ii][0]),float(tagh_tagged_flux[ii][2]))
        
        tagh_energy = float(photon_endpoint[0][0])*(float(tagh_scaled_energy[ii][1]) + 
                                                    float(tagh_scaled_energy[ii][2]))/2.
        
        ps_acc = fPSAcceptance(tagh_energy)
        
        if ps_acc <= 0:
            ps_acc = 10
            
        htagh_flux_cor.SetBinContent(int(tagh_tagged_flux[ii][0]),float(tagh_tagged_flux[ii][1])*ps_scale/ps_acc)
        htagh_flux_cor.SetBinError(int(tagh_tagged_flux[ii][0]),float(tagh_tagged_flux[ii][2])*ps_scale/ps_acc)
                


        data_file_tagh.write("%4d  %10.3f  %10.3f \n" %(int(tagh_tagged_flux[ii][0]),float(tagh_tagged_flux[ii][1])*ps_scale/ps_acc,float(tagh_tagged_flux[ii][2])*ps_scale/ps_acc))


    data_file_tagm = open(OUTPUT_FILE_TAGM,"w+")

    for jj in range(102):
    

        htagm_flux.SetBinContent(int(tagm_tagged_flux[jj][0]),float(tagm_tagged_flux[jj][1]))
        htagm_flux.SetBinError(int(tagm_tagged_flux[jj][0]),float(tagm_tagged_flux[jj][2]))

        tagm_energy = float(photon_endpoint[0][0])*(float(tagm_scaled_energy[jj][1]) + 
                                                    float(tagm_scaled_energy[jj][2]))/2.

        ps_acc = fPSAcceptance(tagm_energy)

        if ps_acc <= 0:
            ps_acc = 10

        htagm_flux_cor.SetBinContent(int(tagm_tagged_flux[jj][0]),float(tagm_tagged_flux[jj][1])*ps_scale/ps_acc)
        htagm_flux_cor.SetBinError(int(tagm_tagged_flux[jj][0]),float(tagm_tagged_flux[jj][2])*ps_scale/ps_acc)


        data_file_tagm.write("%4d  %10.3f  %10.3f \n" %(int(tagm_tagged_flux[jj][0]),float(tagm_tagged_flux[jj][1])*ps_scale/ps_acc,float(tagm_tagged_flux[jj][2])*ps_scale/ps_acc))


    froot_out = TFile(OUTPUT_FILE_ROOT, "recreate")

    htagh_flux.Write()
    htagh_flux_cor.Write()

    htagm_flux.Write()
    htagm_flux_cor.Write()

    froot_out.Close()

    print("Run number: %d"%(FIRST_RUN))
    print("Beam energy %s "%(photon_endpoint[0][0]))
    print(" Output file TAGH:  %s "%(OUTPUT_FILE_TAGH))
    print(" Output file TAGM:  %s "%(OUTPUT_FILE_TAGM))
    print(" Root file:         %s "%(OUTPUT_FILE_ROOT))
    

## main function  
if __name__ == "__main__":
    main()

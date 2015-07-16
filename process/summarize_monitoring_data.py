#!/usr/bin/env python
#
# Run over ROOT files containing monitoring data, and add it to the DB
# TODO: throw exceptions?
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014,2015

from ROOT import TFile,TIter,TDirectory,TH1,TH2,TH1I,TH2I,TCanvas,TFitResultPtr,TF1
from optparse import OptionParser
import os.path
import sys
from array import array
import MySQLdb
import logging

from datamon_db import datamon_db

##########################################################
class summarize_monitoring_data:
    def __init__(self):
        # initialize defaults
        self.VERBOSE = 0
        self.OUTPUT_DIRECTORY = "."

        self.RUN_NUMBER = -1
        self.VERSION_NUMBER = -1
        self.FILE_NUMBER = -1

        self.FILE_NAME = ""
        self.ROOTDIR_PREFIX = ""

        self.mondb = None 
        self.root_file = None

    ##########################################################

    def SumHistContents(self, the_file, hist_path):
        h = the_file.Get(hist_path)
        if(h == None):
            logging.error("Could not load histogram " + hist_path)
            return -1

        sum = 0
        for bin in range(h.GetNbinsX()+1):
            sum += (bin-1)*h.GetBinContent(bin)
        return sum

    # deprecated
    def print_mysql_error(self,e):
        if self.VERBOSE>0:
            try:
                logging.warn("MySQL Error [%d]: %s" % (e.args[0], e.args[1]))
            except IndexError:
                logging.warn("MySQL Error: %s" % str(e))

    # deprecated
    #def CallAndCatchDBErrors(self, func):
    #    try:
    #        func(self.mondb, self.root_file)
    #    except MySQLdb.Error, e:
    #        self.print_mysql_error(e)


    ###########################################
    ## For the CDC, we store the avg number of hits per straw in each superlayer (4 rings)
    ##
    def ProcessCDCHits(self):
        # Store fixed geometry
        nstraws = array('i', [0,42,42,54,54,66,66,80,80,93,93,106,106,123,123,135,135,146,146,158,158,170,170,182,182,197,197,209,209])
        avg_hits_per_superlayer = []
        number_of_events = -1   
    
        cdc_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"CDC/cdc_num_events")
        if(cdc_num_events != None):
            number_of_events = cdc_num_events.GetBinContent(1)

        cdc_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"CDC/cdc_o")
        # sanity checks
        if(cdc_occupancy == None):
            logging.error("couldn't find CDC occupancy histogram!")
            return   

        # calculate average number of hits in each superlayer
        for superlayer in range(len(nstraws)/4):
            nhits = 0
            nstraw = 0
            for sublayer in range(4):
                ring = 4*superlayer+sublayer+1
                nstraw += nstraws[ring]
                for straw in range(1,nstraws[ring]+1):
                    nhits += cdc_occupancy.GetBinContent(straw,ring)
            avg_hits_per_superlayer.append( nhits / nstraw )

        ## insert into DB
        self.mondb.AddCDCHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_superlayer)


    ###########################################
    ## For the Start Counter, we store the avg number of hits per paddle
    ##
    def ProcessSCHits(self):
        avg_hits_per_sector = []
        number_of_events = -1   

        st_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"st/st_num_events")
        if(st_num_events != None):
            number_of_events = st_num_events.GetBinContent(1)

        sc_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"st/st_sec_adc_dhit")
        # sanity checks
        if(sc_occupancy == None):
            logging.error("couldn't find Start Counter occupancy histogram!")
            return   

        for paddle in range(30):
            avg_hits_per_sector.append( sc_occupancy.GetBinContent(paddle+1) )  ## histograms start counting at 1

        ## insert into DB
        self.mondb.AddSCHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_sector)

    ###########################################
    ## For the FDC, we store the avg number of hits per channel in each plane
    ## (note that we might want to correct for the half-length cathode strips)
    ##
    def ProcessFDCHits(self):
        NUM_FDC_STRIPS = 192
        NUM_FDC_WIRES = 96
        avg_hits_per_plane = []
        fdc_avg_hits_per_channel = []
        number_of_events = -1 

        fdc_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"FDC/fdc_num_events")
        if(fdc_num_events != None):
            number_of_events = fdc_num_events.GetBinContent(1)

        plane = 0
        for package in range(1,5):
            for chamber in range(1,7):
                # each chamber has 3 planes: cathode strip/wire/cathode strip
                nhits = 0
                hname = "FDC/Package_%d/fdc_pack%d_chamber%d_upstream_cathode_occ" % (package,package,chamber)
                fdc_strip_occ = self.root_file.Get(self.ROOTDIR_PREFIX+hname)
                if(fdc_strip_occ != None):
                    nhits = fdc_strip_occ.Integral()
                fdc_avg_hits_per_channel.append( nhits/NUM_FDC_STRIPS )

                nhits = 0
                hname = "FDC/Package_%d/fdc_pack%d_chamber%d_wire_occ" % (package,package,chamber)
                fdc_wire_occ = self.root_file.Get(self.ROOTDIR_PREFIX+hname)
                if(fdc_wire_occ != None):
                    nhits = fdc_wire_occ.Integral()
                fdc_avg_hits_per_channel.append( nhits/NUM_FDC_WIRES )

                nhits = 0
                hname = "FDC/Package_%d/fdc_pack%d_chamber%d_downstream_cathode_occ" % (package,package,chamber)
                fdc_strip_occ = self.root_file.Get(self.ROOTDIR_PREFIX+hname)
                if(fdc_strip_occ != None):
                    nhits = fdc_strip_occ.Integral()
                fdc_avg_hits_per_channel.append( nhits/NUM_FDC_STRIPS )

        ## insert into DB
        self.mondb.AddFDCHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, fdc_avg_hits_per_channel)


    ###########################################
    ## For the FCAL, we store the number of hits per channel in the following channels
    ##   4 symmetric about beam pipe:  (27,29)  (31,29)  (29,27)  (29,31)
    ##   4 symmetric midway to edge:   (13,29)  (45,29)  (29,13)  (29,45)
    ##   4 symmetric around edge:      ( 0,29)  (58,29)  (29, 0)  (29,58)
    ##
    def ProcessFCALHits(self):
        hits_per_channel = []
        number_of_events = -1 

        fcal_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"fcal/fcal_num_events")
        if(fcal_num_events != None):
            number_of_events = fcal_num_events.GetBinContent(1)

        fcal_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"fcal/digOcc2D")
        # sanity checks
        if(fcal_occupancy == None):
            logging.error("couldn't find FCAL occupancy histogram!")
            return   

        # the bins are shifted by 2 from the actual row/column numbers
        inner_lo = 27+2
        inner_hi = 31+2
        mid_lo = 13+2
        mid_hi = 45+2
        outer_lo = 0+2
        outer_hi = 58+2
        center = 29+2

        # inner channels
        hits_per_channel.append( fcal_occupancy.GetBinContent(inner_lo,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(inner_hi,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,inner_lo) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,inner_hi) )
        # middle channels
        hits_per_channel.append( fcal_occupancy.GetBinContent(mid_lo,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(mid_hi,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,mid_lo) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,mid_hi) )
        # outer channels
        hits_per_channel.append( fcal_occupancy.GetBinContent(outer_lo,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(outer_hi,center) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,outer_lo) )
        hits_per_channel.append( fcal_occupancy.GetBinContent(center,outer_hi) )

        ## Add padding
        hits_per_channel.append( 0 )
        hits_per_channel.append( 0 )
        hits_per_channel.append( 0 )

        ## insert into DB
        self.mondb.AddFCALHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, hits_per_channel)

    # For FCAL calibrations, store timing information
    def ProcessFCALCalib(self):
        ## calculate calibration info
        calib_vals = []

        # first, check timing
        htime = self.root_file.Get(self.ROOTDIR_PREFIX+"HLDetectorTiming/TRACKING/FCAL - SC Target Time")
        if( htime == None ):
            logging.error("Couldn't find FCAL timing histogram")
            calib_vals += [0., 0.]
        else:
            max = htime.GetBinCenter( htime.GetMaximumBin() )
            # fit within 6 ns of peak
            r = htime.Fit("gaus","SQ", "", -6+max, 6+max)
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)
                calib_vals += [ timing_mean, timing_sigma ]
            else:
                calib_vals += [0., 0.]
                
        # fill DB
        self.mondb.AddFCALCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, calib_vals)


    ###########################################
    ## For the BCAL, we store the avg number of hits and avg energies for the up and down stream channels of
    ## all four layers in four quadrants
    ##
    ## Note: Need to add histograms to get energies out of 
    ##
    def ProcessBCALHits(self):
        avg_hits_per_quadrant = []
        avg_hit_energy_per_quadrant = []
        number_of_events = -1 

        bcal_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal/bcal_num_events")
        if(bcal_num_events != None):
            number_of_events = bcal_num_events.GetBinContent(1)

        bcal_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal/bcal_fadc_occ")
        # sanity checks
        if(bcal_occupancy == None):
            logging.error("couldn't find BCAL occupancy histogram!")
            return   

        for quadrant in range(4):
            for ring in range(12):
                # layer 1
                nhits_layer1_down = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 1)
                nhits_layer1_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 5)
                nhits_layer1_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 9)
                nhits_layer1_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 13)
                nhits_layer1_up = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 18)
                nhits_layer1_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 22)
                nhits_layer1_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 26)
                nhits_layer1_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 30)
                    
                # layer 2
                nhits_layer2_down = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 2)
                nhits_layer2_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 6)
                nhits_layer2_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 10)
                nhits_layer2_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 14)
                nhits_layer2_up = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 19)
                nhits_layer2_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 23)
                nhits_layer2_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 27)
                nhits_layer2_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 31)
            
                # layer 3
                nhits_layer3_down = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 3)
                nhits_layer3_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 7)
                nhits_layer3_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 11)
                nhits_layer3_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 15)
                nhits_layer3_up = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 20)
                nhits_layer3_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 24)
                nhits_layer3_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 28)
                nhits_layer3_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 32)
                
                # layer 4
                nhits_layer4_down = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 4)
                nhits_layer4_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 8)
                nhits_layer4_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 12)
                nhits_layer4_down += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 16)
                nhits_layer4_up = bcal_occupancy.GetBinContent(4*quadrant+ring+1, 21)
                nhits_layer4_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 25)
                nhits_layer4_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 31)
                nhits_layer4_up += bcal_occupancy.GetBinContent(4*quadrant+ring+1, 33)

            avg_hits_per_quadrant += [nhits_layer1_up, nhits_layer1_down, nhits_layer2_up, nhits_layer2_down]
            avg_hits_per_quadrant += [nhits_layer3_up, nhits_layer3_down, nhits_layer4_up, nhits_layer4_down]

        ## insert into DB
        self.mondb.AddBCALHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_quadrant)

        ## monitor BCAL energies, if available
        bcal_energies = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal/bcal_fadc_occ")
        # sanity checks
        if(bcal_energies == None):
            logging.error("couldn't find BCAL energies-per-channel histogram, skipping...")
        else:
            for quadrant in range(4):
                for ring in range(12):
                    # layer 1
                    energies_layer1_down = bcal_energies.GetBinContent(4*quadrant+ring+1, 1)
                    energies_layer1_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 5)
                    energies_layer1_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 9)
                    energies_layer1_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 13)
                    energies_layer1_up = bcal_energies.GetBinContent(4*quadrant+ring+1, 18)
                    energies_layer1_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 22)
                    energies_layer1_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 26)
                    energies_layer1_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 30)
                    
                    # layer 2
                    energies_layer2_down = bcal_energies.GetBinContent(4*quadrant+ring+1, 2)
                    energies_layer2_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 6)
                    energies_layer2_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 10)
                    energies_layer2_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 14)
                    energies_layer2_up = bcal_energies.GetBinContent(4*quadrant+ring+1, 19)
                    energies_layer2_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 23)
                    energies_layer2_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 27)
                    energies_layer2_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 31)
                    
                    # layer 3
                    energies_layer3_down = bcal_energies.GetBinContent(4*quadrant+ring+1, 3)
                    energies_layer3_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 7)
                    energies_layer3_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 11)
                    energies_layer3_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 15)
                    energies_layer3_up = bcal_energies.GetBinContent(4*quadrant+ring+1, 20)
                    energies_layer3_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 24)
                    energies_layer3_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 28)
                    energies_layer3_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 32)
                    
                    # layer 4
                    energies_layer4_down = bcal_energies.GetBinContent(4*quadrant+ring+1, 4)
                    energies_layer4_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 8)
                    energies_layer4_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 12)
                    energies_layer4_down += bcal_energies.GetBinContent(4*quadrant+ring+1, 16)
                    energies_layer4_up = bcal_energies.GetBinContent(4*quadrant+ring+1, 21)
                    energies_layer4_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 25)
                    energies_layer4_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 31)
                    energies_layer4_up += bcal_energies.GetBinContent(4*quadrant+ring+1, 33)

                avg_hit_energy_per_quadrant += [energies_layer1_up, energies_layer1_down, energies_layer2_up, energies_layer2_down]
                avg_hit_energy_per_quadrant += [energies_layer3_up, energies_layer3_down, energies_layer4_up, energies_layer4_down]

            ## insert into DB
            self.mondb.AddBCALEnergies(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hit_energy_per_quadrant)


    # For BCAL, save timing and efficiency information
    def ProcessBCALCalib(self):
        ## calculate calibration info
        calib_vals = []

        # first, check timing
        htime = self.root_file.Get(self.ROOTDIR_PREFIX+"HLDetectorTiming/TRACKING/BCAL - SC Target Time")
        if( htime == None ):
            logging.error("Couldn't find BCAL timing histogram")
            calib_vals += [0., 0.]
        else:
            max = htime.GetBinCenter( htime.GetMaximumBin() )
            # fit within 4 ns of peak
            r = htime.Fit("gaus","SQ", "", -4+max, 4+max)
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)
                calib_vals += [ timing_mean, timing_sigma ]
            else:
                calib_vals += [0., 0.]

        # second, get efficiencies
        bcal_eff = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff_eff")    
        bcal_layer = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff_layer")    
        bcal_layertot = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff_layertot")    
        if( bcal_eff == None or bcal_layer == None or bcal_layertot == None):
            logging.error("Could not find BCAL efficiency histograms")
            calib_vals += [0., 0., 0., 0.]
        else:
            bcal_eff.Divide(bcal_layer,bcal_layertot,1,1,"B")
            calib_vals += [ bcal_eff.GetBinContent(2), bcal_eff.GetBinContent(3), bcal_eff.GetBinContent(4), bcal_eff.GetBinContent(5) ]

        bcal_enhanced_eff = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff2_eff2")    
        bcal_enhanced_layer = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff2_layer")    
        bcal_enhanced_layertot = self.root_file.Get(self.ROOTDIR_PREFIX+"bcal_eff/h1eff2_layertot")    
        if( bcal_enhanced_eff == None or bcal_enhanced_layer == None or bcal_enhanced_layertot == None):
            logging.error("Could not find BCAL enhanced efficiency histograms")
            calib_vals += [0., 0., 0., 0.]
        else:
            bcal_enhanced_eff.Divide(bcal_layer,bcal_enhanced_layertot,1,1,"B")
            calib_vals += [ bcal_enhanced_eff.GetBinContent(2), bcal_enhanced_eff.GetBinContent(3), bcal_enhanced_eff.GetBinContent(4), bcal_enhanced_eff.GetBinContent(5) ]

        # fill DB
        self.mondb.AddBCALCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, calib_vals)


    ###########################################
    ## For the TOF, we store the avg number of hits per some arbitrary grouping
    ##
    def ProcessTOFHits(self):
        avg_hits_per_sector = []
        number_of_events = -1 

        tof_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"tof/tof_num_events")
        if(tof_num_events != None):
            number_of_events = tof_num_events.GetBinContent(1)

        # the TOF occupancy is split into two histos for the two planes
        tof1_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"tof/tofo1")
        tof2_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"tof/tofo2")
        # sanity checks
        if( (tof1_occupancy == None) or (tof2_occupancy == None)):
            logging.error("couldn't find TOF occupancy histogram!")
            return   

        # plane 1
        plane1_up = []
        plane1_down = []
        for group in range(4):
            nhitsup = 0 
            nhitsdown = 0 
            for bar in range(11):
                nhitsup += tof1_occupancy.GetBinContent(4*group+bar+2, 1)
                nhitsdown += tof1_occupancy.GetBinContent(4*group+bar+2, 2)
            plane1_up.append(nhitsup/11)
            plane1_down.append(nhitsdown/11)

        # plane 2
        plane2_up = []
        plane2_down = []
        for group in range(4):
            nhitsup = 0 
            nhitsdown = 0 
            for bar in range(11):
                nhitsup += tof2_occupancy.GetBinContent(1, 4*group+bar+2)
                nhitsdown += tof2_occupancy.GetBinContent(2, 4*group+bar+2)
            plane2_up.append(nhitsup/11)
            plane2_down.append(nhitsdown/11)

        ## insert into DB
        self.mondb.AddTOFHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, plane1_up+plane1_down+plane2_up+plane2_down)

    # keep timing calibrations
    def ProcessTOFCalib(self):
        ## calculate calibration info
        calib_vals = []

        # first, check timing
        htime = self.root_file.Get(self.ROOTDIR_PREFIX+"HLDetectorTiming/TRACKING/TOF - SC Target Time")
        if( htime == None ):
            logging.error("Couldn't find TOF timing histogram")
            calib_vals += [0., 0.]
        else:
            max = htime.GetBinCenter( htime.GetMaximumBin() )
            # fit within 4 ns of peak
            r = htime.Fit("gaus","SQ", "", -6+max, 6+max)
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)
                calib_vals += [ timing_mean, timing_sigma ]
            else:
                calib_vals += [0., 0.]

        # fill DB
        self.mondb.AddTOFCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, calib_vals)



    ###########################################
    ## For the tagger hodoscope, we store the avg number of hits per these groups:
    ##   8-10, 50-52, 110-112, 170-172, 210-212
    ##
    def ProcessTAGHHits(self):
        avg_hits_per_sector = []
        number_of_events = -1 

        tagh_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"TAGH/tagh_num_events")
        if(tagh_num_events != None):
            number_of_events = tagh_num_events.GetBinContent(1)

        # get the occupancy
        tagh_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"TAGH/Hit/Hit_Occupancy")
        # sanity checks
        if( tagh_occupancy == None ): 
            logging.error("couldn't find TAGH occupancy histogram!")
            return   

        # calculate occupancy
        avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(  8)+tagh_occupancy.GetBinContent(  9)+tagh_occupancy.GetBinContent( 10))/3. ]
        avg_hits_per_sector += [ (tagh_occupancy.GetBinContent( 50)+tagh_occupancy.GetBinContent( 51)+tagh_occupancy.GetBinContent( 52))/3. ]
        avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(110)+tagh_occupancy.GetBinContent(111)+tagh_occupancy.GetBinContent(112))/3. ]
        avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(170)+tagh_occupancy.GetBinContent(171)+tagh_occupancy.GetBinContent(172))/3. ]
        avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(210)+tagh_occupancy.GetBinContent(211)+tagh_occupancy.GetBinContent(212))/3. ]

        ## insert into DB
        self.mondb.AddTAGHHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_sector)

    # store timing calibrations, based on PS/Tagger intercalibration
    def ProcessTAGHCalib(self):
        # OLD calibrations
        ## calculate calibration info
        #h = f.Get("HLDetectorTiming/TRACKING/TAGH - SC Target Time")
        #hy = h.ProjectionY()
        #max = hy.GetBinCenter( hy.GetMaximumBin() )
        ## fit within 15 ns of peak
        ##r = hy.Fit("gaus","SQ", "", -4+max, 4+max)
        #fn = TF1("fn", "gaus(0)+[3]", -15+max, 15+max)
        #fn.SetParameter(0,1000)
        #fn.SetParameter(2,1)
        #r = hy.Fit("fn","SQ", "", -15+max, 15+max)
        #timing_mean = r.Parameter(1)
        #timing_sigma = r.Parameter(2)

        timing_mean = 0.
        timing_sigma = 0.
        timing_adc_has_tdc = 0.
        timing_tdc_has_adc = 0.
        # get timing info (only use only channel right now, they aren't aligned enough to sum over multiples)
        tagh_timing = self.root_file.Get(self.ROOTDIR_PREFIX+"PSPair/PSC_PS_TAGH/PSTAGHTimeOffsets_L/PSTAGH_tdiffVsTAGHCounterID_L6")
        if tagh_timing:
            tagh_timing_proj = tagh_timing.ProjectionY("_py",2,2);
            r = tagh_timing_proj.Fit("gaus","SQ");
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)
        tagh_Hit_HasTDCvsHasADC = self.root_file.Get(self.ROOTDIR_PREFIX+"TAGH/Hit/Hit_HasTDCvsHasADC")
        if tagh_Hit_HasTDCvsHasADC:
            if tagh_Hit_HasTDCvsHasADC.GetBinContent(2,1)+tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_adc_has_tdc = tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (tagh_Hit_HasTDCvsHasADC.GetBinContent(2,1)+tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2))
            if tagh_Hit_HasTDCvsHasADC.GetBinContent(1,2)+tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_tdc_has_adc = tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (tagh_Hit_HasTDCvsHasADC.GetBinContent(1,2)+tagh_Hit_HasTDCvsHasADC.GetBinContent(2,2))

        # fill DB
        self.mondb.AddTAGHCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, [timing_mean, timing_sigma, timing_adc_has_tdc, timing_tdc_has_adc])


    ###########################################
    ## For the tagger microscope, we store the avg number of hits per these columns:
    ##   6-8, 24-26, 78-80, 96-98
    ##
    def ProcessTAGMHits(self):
        avg_hits_per_sector = []
        number_of_events = -1 

        tagm_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"tagm/tagm_num_events")
        if(tagm_num_events != None):
            number_of_events = tagm_num_events.GetBinContent(1)

        # get the occupancy
        tagm_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"tagm/tagm_adc_seen")
        # sanity checks
        if( tagm_occupancy == None ): 
            logging.error("couldn't find TAGM occupancy histogram!")
            return   

        # calculate occupancy
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(6) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(7) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(8) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(24) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(25) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(26) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(78) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(79) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(80) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(96) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(97) ]
        avg_hits_per_sector += [ tagm_occupancy.GetBinContent(98) ]
        
        ## insert into DB
        self.mondb.AddTAGMHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_sector)

    # store timing calibrations, based on PS/Tagger intercalibration
    def ProcessTAGMCalib(self):
        ## calculate calibration info
        #h = f.Get("HLDetectorTiming/TRACKING/TAGM - SC Target Time")
        #hy = h.ProjectionY()
        #max = hy.GetBinCenter( hy.GetMaximumBin() )
        ## fit within 15 ns of peak
        ##r = hy.Fit("gaus","SQ", "", -4+max, 4+max)
        #fn = TF1("fn", "gaus(0)+[3]", -15+max, 15+max)
        #fn.SetParameter(0,1000)
        #fn.SetParameter(2,1)
        #r = hy.Fit("fn","SQ", "", -15+max, 15+max)
        #timing_mean = r.Parameter(1)
        #timing_sigma = r.Parameter(2)

        timing_mean = 0.
        timing_sigma = 0.
        # get timing info (only use only channel right now, they aren't aligned enough to sum over multiples)
        tagm_timing = self.root_file.Get(self.ROOTDIR_PREFIX+"PSPair/PSC_PS_TAGM/PSTAGMTimeOffsets_L/PSTAGH_tdiffVsTAGMColumn_L6")
        if tagm_timing:
            tagm_timing_proj = tagm_timing.ProjectionY("_py",2,2);
            r = tagm_timing_proj.Fit("gaus","SQ");
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)

        # fill DB
        self.mondb.AddTAGMCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, [timing_mean, timing_sigma])


    ###########################################
    ## For the coarse PS, we store the hits for all 16 paddles
    ## and some calibration information
    ##
    def ProcessPSCHits(self):
        avg_hits_per_sector = []
        number_of_events = -1 

        psc_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"PSC/psc_num_events")
        if(psc_num_events != None):
            number_of_events = psc_num_events.GetBinContent(1)

        # get the occupancy
        psc_leftarm_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"PSC/Hit/LeftArm/Hit_Occupancy_LeftArm")
        psc_rightarm_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"PSC/Hit/RightArm/Hit_Occupancy_RightArm")
        # sanity checks
        if( psc_leftarm_occupancy == None ): 
            logging.error("couldn't find PSC left arm occupancy histogram!")
            return   
        if( psc_rightarm_occupancy == None ): 
            logging.error("couldn't find PSC right arm occupancy histogram!")
            return   

        # calculate occupancy
        for x in xrange(1,9):
            avg_hits_per_sector += [ psc_leftarm_occupancy.GetBinContent(x) ]
        for x in xrange(1,9):
            avg_hits_per_sector += [ psc_rightarm_occupancy.GetBinContent(x) ]

        ## insert into DB
        self.mondb.AddPSCHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_sector)

    # store timing calibrations, based on PS/Tagger intercalibration
    def ProcessPSCCalib(self):
        ## calculate calibration info
        timing_mean = 0.
        timing_sigma = 0.
        timing_right_adc_has_tdc = 0.
        timing_right_tdc_has_adc = 0.
        timing_left_adc_has_tdc = 0.
        timing_left_tdc_has_adc = 0.
        # get timing info (only use only channel right now, they aren't aligned enough to sum over multiples)
        psc_timing = self.root_file.Get(self.ROOTDIR_PREFIX+"PSPair/PSC/PSCRightArmTimeOffsets/PSC_tdiffVsPSCIDRight_L6")
        if psc_timing:
            psc_timing_proj = psc_timing.ProjectionY("_py",2,2);
            r = psc_timing_proj.Fit("gaus","SQ");
            if int(r) == 0:
                timing_mean = r.Parameter(1)
                timing_sigma = r.Parameter(2)
        psc_Left_Hit_HasTDCvsHasADC = self.root_file.Get(self.ROOTDIR_PREFIX+"PSC/Hit/LeftArm/Hit_HasTDCvsHasADC_LeftArm")
        if psc_Left_Hit_HasTDCvsHasADC:
            if psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,1)+psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_left_adc_has_tdc = psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,1)+psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2))
            if psc_Left_Hit_HasTDCvsHasADC.GetBinContent(1,2)+psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_left_tdc_has_adc = psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (psc_Left_Hit_HasTDCvsHasADC.GetBinContent(1,2)+psc_Left_Hit_HasTDCvsHasADC.GetBinContent(2,2))
        psc_Right_Hit_HasTDCvsHasADC = self.root_file.Get(self.ROOTDIR_PREFIX+"PSC/Hit/RightArm/Hit_HasTDCvsHasADC_RightArm")
        if psc_Right_Hit_HasTDCvsHasADC:
            if psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,1)+psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_right_adc_has_tdc = psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,1)+psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2))
            if psc_Right_Hit_HasTDCvsHasADC.GetBinContent(1,2)+psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2) > 0:
                timing_right_tdc_has_adc = psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2) / (psc_Right_Hit_HasTDCvsHasADC.GetBinContent(1,2)+psc_Right_Hit_HasTDCvsHasADC.GetBinContent(2,2))

        # fill DB
        self.mondb.AddPSCCalib(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, [timing_mean, timing_sigma, timing_left_adc_has_tdc, timing_left_tdc_has_adc,
                                                             timing_right_adc_has_tdc, timing_right_tdc_has_adc ])


    ###########################################
    ## For the fine PS, we store the hits for these 10 counters in each arm:
    ##    1,6,17,45,68,81,105,106,127,145
    ## and some calibration information
    ##
    def ProcessPSHits(self):
        avg_hits_per_sector = []
        number_of_events = -1 

        ps_num_events = self.root_file.Get(self.ROOTDIR_PREFIX+"PS/ps_num_events")
        if(ps_num_events != None):
            number_of_events = ps_num_events.GetBinContent(1)

        # get the occupancy
        ps_leftarm_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"PS/Hit/LeftArm/Hit_Occupancy_LeftArm")
        ps_rightarm_occupancy = self.root_file.Get(self.ROOTDIR_PREFIX+"PS/Hit/RightArm/Hit_Occupancy_RightArm")
        # sanity checks
        if( ps_leftarm_occupancy == None ): 
            logging.error("couldn't find PS left arm occupancy histogram!")
            return   
        if( ps_rightarm_occupancy == None ): 
            logging.error("couldn't find PS right arm occupancy histogram!")
            return   

        # calculate occupancy
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(1) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(6) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(17) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(45) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(68) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(81) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(105) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(106) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(127) ]
        avg_hits_per_sector += [ ps_leftarm_occupancy.GetBinContent(145) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(1) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(6) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(17) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(45) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(68) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(81) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(105) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(106) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(127) ]
        avg_hits_per_sector += [ ps_rightarm_occupancy.GetBinContent(145) ]
        

        ## insert into DB
        self.mondb.AddPSHits(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, avg_hits_per_sector)

    

    ###########################################
    ## Save various count rates for reconstruction-level quantities
    ##
    def ProcessAnalysisInfo(self):
        avg_hits_per_sector = []
        number_of_events = -1   

        num_events = self.root_file.Get("IsEvent")
        if(num_events != None):
            number_of_events = num_events.GetBinContent(2)

        ## extract data generated by the monitoring_hists plugin
        analysis_data = []
    
        num_particles = self.root_file.Get(self.ROOTDIR_PREFIX+"Independent/Hist_TrackMultiplicity/NumReconstructedParticles")
        num_good_particles = self.root_file.Get(self.ROOTDIR_PREFIX+"Independent/Hist_TrackMultiplicity/NumGoodReconstructedParticles")
        # sanity checks
        if(num_particles == None or num_good_particles == None):
            logging.error("couldn't find Analysis particle count histograms!")
            return   

        # Calculate number of reconstructed particles (q+,q-,p,pi+,pi-,K+)
        num_partplus = 0
        num_partminus = 0
        num_proton = 0
        num_piplus = 0
        num_piminus = 0
        num_kplus = 0
        num_neutral = 0
        for bin in range(num_particles.GetNbinsY()+1):
            numpart = bin-1
            num_partplus  += numpart*num_particles.GetBinContent(4,bin)
            num_partminus += numpart*num_particles.GetBinContent(5,bin)
            num_proton    += numpart*num_particles.GetBinContent(9,bin)
            num_piplus    += numpart*num_particles.GetBinContent(7,bin)
            num_piminus   += numpart*num_particles.GetBinContent(10,bin)
            num_kplus     += numpart*num_particles.GetBinContent(8,bin)
            num_neutral  += numpart*num_particles.GetBinContent(3,bin)
            analysis_data += [num_partplus,num_partminus,num_proton,num_piplus,num_piminus,num_kplus]

        # Calculate number of GOOD reconstructed particles (q+,q-,p,pi+,pi-,K+)
        num_good_partplus = 0
        num_good_partminus = 0
        num_good_proton = 0
        num_good_piplus = 0
        num_good_piminus = 0
        num_good_kplus = 0
        num_good_neutral = 0
        for bin in range(num_good_particles.GetNbinsY()+1):
            numpart = bin-1
            num_good_partplus  += numpart*num_good_particles.GetBinContent(4,bin)
            num_good_partminus += numpart*num_good_particles.GetBinContent(5,bin)
            num_good_proton    += numpart*num_good_particles.GetBinContent(9,bin)
            num_good_piplus    += numpart*num_good_particles.GetBinContent(7,bin)
            num_good_piminus   += numpart*num_good_particles.GetBinContent(10,bin)
            num_good_kplus     += numpart*num_good_particles.GetBinContent(8,bin)
            num_good_neutral  += numpart*num_good_particles.GetBinContent(3,bin)
        analysis_data += [num_good_partplus,num_good_partminus,num_good_proton,num_good_piplus,num_good_piminus,num_good_kplus]

        # add in neutral particles
        analysis_data += [num_neutral,num_good_neutral]

        # Calculate number of various other reconstructed quantities    
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumFCALShowers") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumBCALShowers") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTOFPoints") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumSCHits") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTAGHHits") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTAGMHits") )
        
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTrackBCALMatches") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTrackFCALMatches") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTrackTOFMatches") )
        analysis_data.append( self.SumHistContents(self.root_file, self.ROOTDIR_PREFIX+"Independent/Hist_NumReconstructedObjects/NumTrackSCMatches") )

        ## insert into DB
        self.mondb.AddAnalysisInfo(self.RUN_NUMBER, self.FILE_NUMBER, self.VERSION_NUMBER, number_of_events, analysis_data)


    # do processing for results from a single EVIO file
    def ProcessFile(self):
        # check inputs
        if(not os.path.isfile(self.FILE_NAME)):
            logging.error("no such file: " + self.FILE_NAME)
            return
        if(self.RUN_NUMBER < 1):
            logging.error("bad run number! = " + str(self.RUN_NUMBER))
            return
        if(self.VERSION_NUMBER < 1):
            logging.error("bad version number! = " + str(self.VERSION_NUMBER))
            return
        if(self.FILE_NUMBER < 0):
            logging.error("bad file number! = " + str(self.FILE_NUMBER))
            return

        # initializing monitoring DB connection
        self.mondb = datamon_db()
        # open input file 
        self.root_file = TFile(self.FILE_NAME)

        # keep summary info about the hits in different subdetectors
        self.ProcessCDCHits()
        self.ProcessFDCHits()
        self.ProcessFCALHits()
        self.ProcessBCALHits()
        self.ProcessSCHits()
        self.ProcessTOFHits()
        self.ProcessTAGHHits()
        self.ProcessTAGMHits()
        self.ProcessPSHits()
        self.ProcessPSCHits()

        self.ProcessAnalysisInfo()

        # cleanup
        #self.root_file.Close()
        del self.root_file
        del self.mondb


    # do processing for results for a whole run 
    # only save calibration results for the whole run
    def ProcessRun(self):
        # check inputs
        if(not os.path.isfile(self.FILE_NAME)):
            logging.error("no such file: " + self.FILE_NAME)
            return
        if(self.RUN_NUMBER < 1):
            logging.error("bad run number! = " + str(self.RUN_NUMBER))
            return
        if(self.VERSION_NUMBER < 1):
            logging.error("bad version number! = " + str(self.VERSION_NUMBER))
            return

        # for full runs, fix the file number to -1
        self.FILE_NUMBER = -1

        # initializing monitoring DB connection
        self.mondb = datamon_db()
        # open input file 
        self.root_file = TFile(self.FILE_NAME)

        # keep summary info about the hits in different subdetectors
        self.ProcessCDCHits()
        self.ProcessFDCHits()
        self.ProcessFCALHits()
        self.ProcessBCALHits()
        self.ProcessSCHits()
        self.ProcessTOFHits()
        self.ProcessTAGHHits()
        self.ProcessTAGMHits()
        self.ProcessPSHits()
        self.ProcessPSCHits()

        self.ProcessAnalysisInfo()

        # keep summary info on calibrations
        self.ProcessFCALCalib()
        self.ProcessBCALCalib()
        self.ProcessTOFCalib()
        self.ProcessTAGHCalib()
        self.ProcessTAGMCalib()
        self.ProcessPSCCalib()

        # cleanup
        #self.root_file.Close()
        del self.root_file
        del self.mondb



if __name__ == "__main__":
    # read in command line args
    parser = OptionParser(usage = "summarize_monitoring_data.py [options] run_number version_number file.root")
    parser.add_option("-D","--output_dir", dest="output_dir",
                      help="Directory where output files will be stored")
    parser.add_option("-F","--file_number", dest="file_number",
                      help="Specify the file number in the run that we are monitoring")
    parser.add_option("-R","--root_dir", dest="root_dir",
                      help="The base ROOT directory in which the histograms are stored")

    (options, args) = parser.parse_args(sys.argv)

    if(len(args) < 3):
        parser.print_help()
        sys.exit(0)

    # create main object
    summarizer = summarize_monitoring_data()

    # configure class
    # needs more error checking
    run_number = int(args[0])
    version_number = int(args[1])
    summarizer.FILE_NAME = str(args[2])

    if(options.file_number):
        summarizer.FILE_NUMBER = options.file_number
    if(options.output_dir):
        if(os.path.isdir(options.output_dir)):
            summarizer.OUTPUT_DIRECTORY = options.output_dir
        else:
            print "WARNING: Output directory '"+options.output_dir+"' does not exist, defaulting to current directory..."
    if(options.root_dir):
        summarizer.ROOTDIR_PREFIX = options.root_dir

    # sanity checks
    if(run_number > 0):
        summarizer.RUN_NUMBER = run_number
    if(version_number > 0):
        summarizer.VERSION_NUMBER = version_number

    # do the actual work
    # if the file number is specified, run the file processing, otherwise do the run processing
    if options.file_number:  
        summarizer.ProcessFile()
    else:
        summarizer.ProcessRun()

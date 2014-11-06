#!/usr/bin/env python
#
# Run over ROOT files containing monitoring data, and add it to the DB
# TODO: throw exceptions?
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014

from ROOT import TFile,TIter,TDirectory,TH1,TH2,TH1I,TH2I,TCanvas
from optparse import OptionParser
import os.path
import sys
from array import array

from datamon_db import datamon_db

##########################################################
## Globals
##########################################################

OUTPUT_DIRECTORY = "."

RUN_NUMBER = -1
VERSION_NUMBER = -1
FILE_NUMBER = 1

ROOTDIR_PREFIX = "rootspy/"

# the canvas
#c1  = TCanvas("c1","",CANVAS_WIDTH,CANVAS_HEIGHT)


##########################################################


def SumHistContents(root_file, hist_path):
    h = root_file.Get(hist_path)
    if(h == None):
        print "Could not load " + hist_path
        return -1

    sum = 0
    for bin in range(h.GetNbinsX()+1):
        sum += (bin-1)*h.GetBinContent(bin)
    return sum
             


###########################################
## For the CDC, we store the avg number of hits per straw in each superlayer (4 rings)
## and some calibration information
##
def ProcessCDC(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER,ROOTDIR_PREFIX

    nstraws = array('i', [0,42,42,54,54,66,66,80,80,93,93,106,106,123,123,135,135,146,146,158,158,170,170,182,182,197,197,209,209])
    avg_hits_per_superlayer = []
    number_of_events = -1   
    
    cdc_num_events = root_file.Get(ROOTDIR_PREFIX+"CDC/cdc_num_events")
    if(cdc_num_events != None):
        number_of_events = cdc_num_events.GetBinContent(1)

    cdc_occupancy = root_file.Get(ROOTDIR_PREFIX+"CDC/cdc_o")
    # sanity checks
    if(cdc_occupancy == None):
        print "couldn't find CDC occupancy histogram!"
        return   

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
    db.AddCDCHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hits_per_superlayer)

    ## calculate calibration info

###########################################
## For the Start Counter, we store the avg number of hits per paddle
## and some calibration information
##
def ProcessSC(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER,ROOTDIR_PREFIX

    avg_hits_per_sector = []
    number_of_events = -1   

    st_num_events = root_file.Get(ROOTDIR_PREFIX+"st/st_num_events")
    if(st_num_events != None):
        number_of_events = st_num_events.GetBinContent(1)

    sc_occupancy = root_file.Get(ROOTDIR_PREFIX+"st/st_sec_adc_dhit")
    # sanity checks
    if(sc_occupancy == None):
        print "couldn't find Start Counter occupancy histogram!"
        return   

    for paddle in range(30):
        avg_hits_per_sector.append( sc_occupancy.GetBinContent(paddle+1) )  ## histograms start counting at 1

    ## insert into DB
    db.AddSCHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hits_per_sector)

    ## calculate calibration info

###########################################
## For the FDC, we store the avg number of hits per channel in each plane
## (note that we might want to correct for the half-length cathode strips)
## and some calibration information
##
def ProcessFDC(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_plane = []
    number_of_events = -1 
    NUM_FDC_STRIPS = 192
    NUM_FDC_WIRES = 96

    fdc_num_events = root_file.Get(ROOTDIR_PREFIX+"FDC/fdc_num_events")
    if(fdc_num_events != None):
        number_of_events = fdc_num_events.GetBinContent(1)

    fdc_strip_occupancy_2d = root_file.Get(ROOTDIR_PREFIX+"FDC/fdcos")
    fdc_wire_occupancy_2d = root_file.Get(ROOTDIR_PREFIX+"FDC/fdcow")
    # sanity checks
    if( (fdc_strip_occupancy_2d==None) or (fdc_wire_occupancy_2d==None) ):
        print "couldn't find FDC occupancy histograms!"
        return   

    fdc_strip_occupancy = fdc_strip_occupancy_2d.ProjectionY()
    fdc_wire_occupancy = fdc_wire_occupancy_2d.ProjectionY()
    fdc_hits_per_channel = TH1I("fdco","FDC occupancy by plane",72,1,72)

    for layer in range(72):
#        print " FDC layer " + str(layer+1) + " = " + str(fdc_strip_occupancy.GetBinContent(layer+1) / NUM_FDC_STRIPS)
        fdc_hits_per_channel.SetBinContent(layer+1, fdc_strip_occupancy.GetBinContent(layer+1) / NUM_FDC_STRIPS)
    for wirelayer in range(24):
#        print " FDC layer " + str(3*wirelayer+2) + " = " + str(fdc_wire_occupancy.GetBinContent(wirelayer+1) / NUM_FDC_WIRES)
        fdc_hits_per_channel.SetBinContent(3*wirelayer+2, fdc_wire_occupancy.GetBinContent(wirelayer+1) / NUM_FDC_WIRES)
        
    ## insert into DB
    db.AddFDCHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, [fdc_hits_per_channel.GetBinContent(x+1) for x in range(72)])

    ## calculate calibration info

###########################################
## For the FCAL, we store the number of hits per channel in the following channels
##   4 symmetric about beam pipe:  (27,29)  (31,29)  (29,27)  (29,31)
##   4 symmetric midway to edge:   (13,29)  (45,29)  (29,13)  (29,45)
##   4 symmetric around edge:      ( 0,29)  (58,29)  (29, 0)  (29,58)
## and some calibration information
##
def ProcessFCAL(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    hits_per_channel = []
    number_of_events = -1 

    fcal_num_events = root_file.Get(ROOTDIR_PREFIX+"fcal/fcal_num_events")
    if(fcal_num_events != None):
        number_of_events = fcal_num_events.GetBinContent(1)

    fcal_occupancy = root_file.Get(ROOTDIR_PREFIX+"fcal/digOcc2D")
    # sanity checks
    if(fcal_occupancy == None):
        print "couldn't find FCAL occupancy histogram!"
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
    db.AddFCALHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, hits_per_channel)

    ## calculate calibration info

###########################################
## For the BCAL, we store the avg number of hits and avg energies for the up and down stream channels of
## all four layers in four quadrants
## and some calibration information
##
## Note: Need to add histograms to get energies out of
##
def ProcessBCAL(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_quadrant = []
    avg_hit_energy_per_quadrant = []
    number_of_events = -1 

    bcal_num_events = root_file.Get(ROOTDIR_PREFIX+"bcal/bcal_num_events")
    if(bcal_num_events != None):
        number_of_events = bcal_num_events.GetBinContent(1)

    bcal_occupancy = root_file.Get(ROOTDIR_PREFIX+"bcal/bcal_fadc_occ")
    # sanity checks
    if(bcal_occupancy == None):
        print "couldn't find BCAL occupancy histogram!"
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
    db.AddBCALHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hits_per_quadrant)

    ## monitor BCAL energies, if available
    bcal_energies = root_file.Get(ROOTDIR_PREFIX+"bcal/bcal_fadc_occ")
    # sanity checks
    if(bcal_energies == None):
        print "couldn't find BCAL energies-per-channel histogram, skipping..."
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
        db.AddBCALEnergies(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hit_energy_per_quadrant)


                    
    ## calculate calibration info

###########################################
## For the TOF, we store the avg number of hits per some arbitrary grouping
## and some calibration information
##
def ProcessTOF(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_sector = []
    number_of_events = -1 

    tof_num_events = root_file.Get(ROOTDIR_PREFIX+"tof/tof_num_events")
    if(tof_num_events != None):
        number_of_events = tof_num_events.GetBinContent(1)

    # the TOF occupancy is split into two histos for the two planes
    tof1_occupancy = root_file.Get(ROOTDIR_PREFIX+"tof/tofo1")
    tof2_occupancy = root_file.Get(ROOTDIR_PREFIX+"tof/tofo2")
    # sanity checks
    if( (tof1_occupancy == None) or (tof2_occupancy == None)):
        print "couldn't find TOF occupancy histogram!"
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
    db.AddTOFHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, plane1_up+plane1_down+plane2_up+plane2_down)

    ## calculate calibration info


###########################################
## For the tagger hodoscope, we store the avg number of hits per these groups:
##   8-10, 50-52, 110-112, 170-172, 210-212
## and some calibration information
##
def ProcessTAGH(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_sector = []
    number_of_events = -1 

    tagh_num_events = root_file.Get(ROOTDIR_PREFIX+"TAGH/tagh_num_events")
    if(tagh_num_events != None):
        number_of_events = tagh_num_events.GetBinContent(1)

    # get the occupancy
    tagh_occupancy = root_file.Get(ROOTDIR_PREFIX+"TAGH/Hit_NHits")
    # sanity checks
    if( tagh_occupancy == None ): 
        print "couldn't find TAGH occupancy histogram!"
        return   

    # calculate occupancy
    avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(  8)+tagh_occupancy.GetBinContent(  9)+tagh_occupancy.GetBinContent( 10))/3. ]
    avg_hits_per_sector += [ (tagh_occupancy.GetBinContent( 50)+tagh_occupancy.GetBinContent( 51)+tagh_occupancy.GetBinContent( 52))/3. ]
    avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(110)+tagh_occupancy.GetBinContent(111)+tagh_occupancy.GetBinContent(112))/3. ]
    avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(170)+tagh_occupancy.GetBinContent(171)+tagh_occupancy.GetBinContent(172))/3. ]
    avg_hits_per_sector += [ (tagh_occupancy.GetBinContent(210)+tagh_occupancy.GetBinContent(211)+tagh_occupancy.GetBinContent(212))/3. ]

    ## insert into DB
    db.AddTAGHHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hits_per_sector)

    ## calculate calibration info

###########################################
## For the tagger microscope, we store the avg number of hits per these columns:
##   6-8, 24-26, 78-80, 96-98
## and some calibration information
##
def ProcessTAGM(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_sector = []
    number_of_events = -1 

    tagm_num_events = root_file.Get(ROOTDIR_PREFIX+"tagm/tagm_num_events")
    if(tagm_num_events != None):
        number_of_events = tagm_num_events.GetBinContent(1)

    # get the occupancy
    tagm_occupancy = root_file.Get(ROOTDIR_PREFIX+"tagm/tagm_adc_seen")
    # sanity checks
    if( tagm_occupancy == None ): 
        print "couldn't find TAGM occupancy histogram!"
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
    db.AddTAGMHits(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, avg_hits_per_sector)

    ## calculate calibration info



###########################################
## Save various count rates for reconstruction-level quantities
##
def ProcessAnalysisInfo(db, root_file):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER

    avg_hits_per_sector = []
    number_of_events = -1   

    num_events = root_file.Get("IsEvent")
    if(num_events != None):
        number_of_events = num_events.GetBinContent(2)

    ## extract data generated by the monitoring_hists plugin
    analysis_data = []
    
    num_particles = root_file.Get(ROOTDIR_PREFIX+"Independent/Hist_TrackMultiplicity/NumReconstructedParticles")
    num_good_particles = root_file.Get(ROOTDIR_PREFIX+"Independent/Hist_TrackMultiplicity/NumGoodReconstructedParticles")
    # sanity checks
    if(num_particles == None or num_good_particles == None):
        print "couldn't find Analysis particle count histograms!"
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
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumFCALShowers") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumBCALShowers") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTOFPoints") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumSCHits") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTAGHHits") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTAGMHits") )

    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTrackBCALMatches") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTrackFCALMatches") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTrackTOFMatches") )
    analysis_data.append( SumHistContents(root_file, "Independent/Hist_NumReconstructedObjects/NumTrackSCMatches") )

    ## insert into DB
    db.AddAnalysisInfo(RUN_NUMBER, VERSION_NUMBER, FILE_NUMBER, number_of_events, analysis_data)




def main(argv):
    global RUN_NUMBER,VERSION_NUMBER,FILE_NUMBER
    
    # read in command line args
    parser = OptionParser(usage = "process_monitoring_data.py [options] run_number version_number file.root")
    parser.add_option("-D","--output_dir", dest="output_dir",
                      help="Directory where output files will be stored")
    parser.add_option("-F","--file_number", dest="file_number",
                      help="Specify the file number in the run that we are monitoring")

    (options, args) = parser.parse_args(argv)

    if(len(args) < 3):
        parser.print_help()
        exit(0)


    run_number = int(args[0])
    version_number = int(args[1])
    file_name = str(args[2])

    if(options.file_number):
        FILE_NUMBER = options.file_number
    if(options.output_dir):
        if(os.path.isdir(options.output_dir)):
            OUTPUT_DIRECTORY = options.output_dir
        else:
            print "WARNING: Output directory '"+options.output_dir+"' does not exist, defaulting to current directory..."
        
    # sanity checks
    if(run_number > 0):
        RUN_NUMBER = run_number
    if(version_number > 0):
        VERSION_NUMBER = version_number

    if(not os.path.isfile(file_name)):
        print " no such file: " + file_name
    if(RUN_NUMBER < 1):
        print " bad run number! = " + str(RUN_NUMBER)
    if(VERSION_NUMBER < 1):
        print " bad version number! = " + str(VERSION_NUMBER)
    if(FILE_NUMBER < 1):
        print " bad file number! = " + str(FILE_NUMBER)

    
    # initializing monitoring DB connection
    mondb = datamon_db()

    # run over data to make some plots
    root_file = TFile(file_name)

    ## calculate number of events??

    # Do subdetector-specific tasks
    ProcessCDC(mondb, root_file)
    ProcessFDC(mondb, root_file)
    ProcessFCAL(mondb, root_file)
    ProcessBCAL(mondb, root_file)
    ProcessSC(mondb, root_file)
    ProcessTOF(mondb, root_file)
    ProcessTAGH(mondb, root_file)
    ProcessTAGM(mondb, root_file)

    # Process higher-level data
    ProcessAnalysisInfo(mondb, root_file)

    # cleanup
    root_file.Close()


if __name__ == "__main__":
    main(sys.argv[1:])

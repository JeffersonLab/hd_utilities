#!/usr/bin/python

# This script calculates the various timing offsets for the CCDB.
# Created by Alex Barnes, October 18, 2016

# Usage:
# python timing.py -b <root file> <run number> <calibration type>
# The calibration type can be: self or rf,
# where self refers to TDC-ADC
# Specifying the calibration type determines which histogram
# the script uses for the calibration.
##################################################################

import os,sys
from ROOT import *

# Set up CCDB connection for reading tables
import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet

# CCDB_HOME and CCDB_CONNECTION environment variables must be set!
ccdb_home = os.environ["CCDB_HOME"]
sqlite_connect_str = os.environ["CCDB_CONNECTION"]

# Create CCDB API class
## This class has all CCDB manipulation functions
provider = ccdb.AlchemyProvider()
## Use usual connection string to connect to database
provider.connect(sqlite_connect_str)
print 'CCDB is connected? ' + str(getattr(provider,"is_connected"))
## Provide a username for CCDB updates
provider.authentication.current_user_name = "anonymous"	
## Set the default CCDB variation
global variation; variation = 'calib'

# Define TAGM constants
global NCOLUMNS; NCOLUMNS = 102
global NROWS; NROWS = 5

def main():
	if (len(sys.argv) < 5):
		print "\nUsage: python timing.py -b <root file> <run number> <calibration type> <CCDB variation>"
		print "The calibration type can be: self, rf, res, or validate,"
		print "where self refers to TDC-ADC and res gives the resolution\n"
		print "Example CCDB variations are: default, calib\n"
		return

	outroot = TFile.Open('resolution.root','RECREATE')
	rootfile = TFile.Open(str(sys.argv[2]))
	run = sys.argv[3]
	calib_type = str(sys.argv[4])
	if (len(sys.argv) > 5):
		globals()['variation'] = str(sys.argv[5])
	print 'Variation to be used: ' + variation
	adcfile = open('adc_offsets-' + str(run) + '.txt','w')
	tdcfile = open('tdc_offsets-' + str(run) + '.txt','w')
	offset_file = open('offsets-' + str(run) + '.txt', 'w')
	errorfile = open('problem-channels.txt','w')

	baseDir = 'TAGM_TW/'

	beamPeriod_assignment = provider.get_assignment("/PHOTON_BEAM/RF/beam_period",run,variation)
	beamPeriod = float(beamPeriod_assignment.constant_set.data_table[0][0])
	print "The beam period is: " + str(beamPeriod)
	fadc_assignment = provider.get_assignment("/PHOTON_BEAM/microscope/fadc_time_offsets",run,variation)
	tdc_assignment = provider.get_assignment("/PHOTON_BEAM/microscope/tdc_time_offsets",run,variation)

	ind_cols = [9, 27, 81, 99]
	channel = 0
	res_hist = TProfile("FWHM","Timing resolution (FWHM) of each TAGM channel;Channel;Resolution [ns]",122,1,123)
	res_hist2 = TProfile("1sig","Timing resolution (1 sigma) of each TAGM channel;Channel;Resolution [ns]",122,1,123)
	for i in range(1,NCOLUMNS+1):
		# Get summed channels
		init_adc = float(fadc_assignment.constant_set.data_table[channel][2])
		if (abs(init_adc) > 50):
			errorfile.write('Initial fadc offset for col ' + str(i) + ' is too large. Manually adjust and re-run plugin before proceeding\n')
			print('Initial fadc offset for col ' + str(i) + ' is too large. Manually adjust and re-run plugin before proceeding\n')
		init_tdc = float(tdc_assignment.constant_set.data_table[channel][2])
		if (abs(init_tdc) > 50):
			errorfile.write('Initial tdc offset for col ' + str(i) + ' is too large. Manually adjust and re-run plugin before proceeding\n')
			print('Initial tdc offset for col ' + str(i) + ' is too large. Manually adjust and re-run plugin before proceeding\n')

		if (calib_type == 'self'):
			h_name = baseDir+"t_adc_all"
			hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(i),i,i)
			new_offset = GetSelfTiming(hist)
		elif (calib_type == 'rf'):
			h_name = baseDir+"tdc_adc_all"
			hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(i),i,i)
			tdc_offset = GetSelfTiming(hist)

			h_name = baseDir+"adc_rf_all"
			hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(i),i,i)
			new_offset = GetRFTiming(hist,beamPeriod)
		elif (calib_type == 'res'):
			h_name = baseDir+"t-rf/h_dt_vs_pp_"
			hist = rootfile.Get(h_name+str(i)).Clone().ProjectionY()
			res = GetResolution(hist)
			res_hist.Fill(i,res)
			res_hist2.Fill(i,res/2.355)
		elif (calib_type == 'validate'):
			h_name = baseDir+"adc_rf_all"
			hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(i),i,i)
			mean = GetRFTiming(hist,beamPeriod)
			if (abs(mean) > 0.05):
				errorfile.write(MeanError('ADC-RF',0,i,mean))

			h_name = baseDir+"t_adc_all"
			hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(i),i,i)
			mean = GetSelfTiming(hist)
			if (abs(mean) > 0.05):
				errorfile.write(MeanError('Self',0,i,mean))

			h_name = baseDir+"t-rf/h_dt_vs_pp_"
			hist = rootfile.Get(h_name+str(i)).Clone().ProjectionY()
			mean = GetRFTiming(hist,beamPeriod)
			sig = GetResolution(hist)/2.355
			if (abs(mean) > 0.05):
				errorfile.write(MeanError('TDC-RF',0,i,mean))
			if (sig > 1.0):
				errorfile.write(SigError('TDC-RF',0,i,sig))
		else:
			print "\nIncorrect calibration type. Please use self, rf, res, or validate.\n"
			return

		# Write offset to file
		if (calib_type == 'self'):
			offset = float(tdc_assignment.constant_set.data_table[channel][2]) + new_offset
			if (abs(offset) > 50):
				errorfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')
				tdcfile.write(' 0\t' + str(i) + '\t' + '10.0' + '\n')
			else:
				tdcfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')
		elif (calib_type == 'rf'):
			offset = float(fadc_assignment.constant_set.data_table[channel][2]) + new_offset
			if (abs(offset) > 50):
				errorfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')
				adcfile.write(' 0\t' + str(i) + '\t' + '10.0' + '\n')
			else:
				adcfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')

			offset = float(tdc_assignment.constant_set.data_table[channel][2]) + tdc_offset + new_offset
			if (abs(offset) > 50):
				errorfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')
				tdcfile.write(' 0\t' + str(i) + '\t' + '10.0' + '\n')
			else:
				tdcfile.write(' 0\t' + str(i) + '\t' + str(offset) + '\n')
			offset_file.write(' 0\t' + str(i) + '\t' + str(tdc_offset + new_offset) + '\n')

		channel += 1

		# Get individual channels
		if i in ind_cols:
			for j in range(5):
				id = ind_cols.index(i)*5+j+1
				if (calib_type == 'self'):
					h_name = baseDir+"t_adc_all_ind"
					hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(id),id,id)
					new_offset = GetSelfTiming(hist)
				elif (calib_type == 'rf'):
					h_name = baseDir+"tdc_adc_all_ind"
					hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(id),id,id)
					tdc_offset = GetSelfTiming(hist)

					h_name = baseDir+"adc_rf_all_ind"
					hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(id),id,id)
					new_offset = GetRFTiming(hist,beamPeriod)
				elif (calib_type == 'res'):
					h_name = baseDir+"t-rf/h_dt_vs_pp_ind_"
					hist = rootfile.Get(h_name+str(j+1)+"_"+str(ind_cols.index(i)+1)).Clone().ProjectionY()
					res = GetResolution(hist)
					res_hist.Fill(102+id,res)
					res_hist2.Fill(102+id,res/2.355)
				elif (calib_type == 'validate'):
					h_name = baseDir+"adc_rf_all_ind"
					hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(id),id,id)
					mean = GetRFTiming(hist,beamPeriod)
					if (abs(mean) > 0.05):
						errorfile.write(MeanError('ADC-RF',j+1,i,mean))

					h_name = baseDir+"t_adc_all_ind"
					hist = rootfile.Get(h_name).Clone().ProjectionX(h_name+"_"+str(id),id,id)
					mean = GetSelfTiming(hist)
					if (abs(mean) > 0.05):
						errorfile.write(MeanError('Self',j+1,i,mean))

					h_name = baseDir+"t-rf/h_dt_vs_pp_ind_"
					hist = rootfile.Get(h_name+str(j+1)+"_"+str(ind_cols.index(i)+1)).Clone().ProjectionY()
					mean = GetRFTiming(hist,beamPeriod)
					sig = GetResolution(hist)
					if (abs(mean) > 0.05):
						errorfile.write(MeanError('TDC-RF',j+1,i,mean))
					if (sig > 1.0):
						errorfile.write(SigError('TDC-RF',j+1,i,sig))
				else:
					print "\nIncorrect calibration type. Please use self, rf, res, or validate.\n"
					return

				# Write offset to file
				if (calib_type == 'self'):
					offset = float(tdc_assignment.constant_set.data_table[channel][2]) + new_offset
					if (abs(offset) > 50):
						errorfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')
						tdcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + '10.0' + '\n')
					else:
						tdcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')
				elif (calib_type == 'rf'):
					offset = float(fadc_assignment.constant_set.data_table[channel][2]) + new_offset
					if (abs(offset) > 50):
						errorfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')
						adcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + '10.0' + '\n')
					else:
						adcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')

					offset = float(tdc_assignment.constant_set.data_table[channel][2]) +tdc_offset + new_offset
					if (abs(offset) > 50):
						errorfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')
						tdcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + '10.0' + '\n')
					else:
						tdcfile.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(offset) + '\n')
					offset_file.write(' ' + str(j+1) + '\t' + str(i) + '\t' + str(tdc_offset + new_offset) + '\n')

				channel += 1

	outroot.cd()
	res_hist.Write()
	res_hist2.Write()

	adcfile.close()
	tdcfile.close()
	offset_file.close()
	errorfile.close()
	rootfile.Close()
	outroot.Close()

def GetRFTiming(hist,beamPeriod):
	# This method will take in a histogram and return the histogram's offset from 0
	histname = hist.GetName()
	if (hist.GetMaximum() < 5):
		print 'Insufficient number of entries for histogram ' + histname
		return 0
	if (hist.GetEntries() < 10000):
		hist.RebinX(4)
	maximum = GetMaximum(hist)
	try:
		if (maximum < -0.5):
			fitfunc = TF1("fitfunc","[0]*TMath::Exp(-0.5*pow((x-[1])/[2],2))+[0]*TMath::Exp(-0.5*pow((x-[1]-[3])/[2],2))",-beamPeriod/2,0)
			fitMin = -beamPeriod/2
			fitMax = maximum + 0.5
		elif (maximum > 0.5):
			fitfunc = TF1("fitfunc","[0]*TMath::Exp(-0.5*pow((x-[1])/[2],2))+[0]*TMath::Exp(-0.5*pow((x-[1]+[3])/[2],2))",0,beamPeriod/2)
			fitMin = maximum - 0.5
			fitMax = beamPeriod/2
		else:
			fitfunc = TF1("fitfunc","[0]*TMath::Exp(-0.5*pow((x-[1])/[2],2))",-beamPeriod/2,beamPeriod/2)
			fitMin = maximum - 0.4
			fitMax = maximum + 0.4
		fitfunc.SetParameter(1,maximum)
		fitfunc.SetParLimits(1,-beamPeriod/2,beamPeriod/2)
		fitfunc.SetParameter(2,0.4)
		if (abs(maximum) > 0.5):
			fitfunc.FixParameter(3,beamPeriod)
		FitResult = hist.Fit(fitfunc,"sRq","",fitMin,fitMax)
			
		offset = FitResult.Parameters()[1]
		sigma = FitResult.Parameters()[2]
	except:
		print "fit failed for histogram " + histname
		offset = 0
	
	return offset

def GetSelfTiming(hist):
	# This method will take in a histogram and return the histogram's offset from 0
	histname = hist.GetName()
	if (hist.GetMaximum() < 5):
		print 'Insufficient number of entries for histogram ' + histname
		return 0
	maximum = GetMaximum(hist)
	if (hist.GetEntries() < 10000):
		hist.RebinX(4)
	try:
		FitResult = hist.Fit("gaus","sRWq","",maximum-0.5,maximum+0.5)
		offset = FitResult.Parameters()[1]
		sigma = FitResult.Parameters()[2]
	except:
		print "fit failed for histogram " + histname
		offset = 0
	
	return offset

def GetResolution(hist):
	# This method fits the histogram with a Gaussian and returns the FWHM
	histname = hist.GetName()
	if (hist.GetMaximum() < 5):
		print 'Insufficient number of entries for histogram ' + histname
		return 0
	try:
		FitResult = hist.Fit("gaus","sRWq","",-1,1)
		sigma = FitResult.Parameters()[2]
		FWHM = 2.355*sigma
	except:
		print 'Fit failed for histogram ' + histname
		FWHM = 0

	return FWHM

def MeanError(type,row,col,mean):
	if (row == 0):
		meanError = type + ': column ' + str(col) + ' has a |mean| (' + str(mean) + ') greater than 0.05 ns\n'
	else:
		meanError = type + ': Individual column ' + str(col) + ' row ' + str(row) + \
			    ' has a |mean| (' + str(mean) + ') greater than 0.05 ns\n'
	return meanError

def SigError(type,row,col,sig):
	if (row == 0):
		sigError = type + ': column ' + str(col) + ' has a sigma (' + str(sig) + ') greater than 1.0 ns\n'
	else:
		sigError = type + ': Individual column ' + str(col) + ' row ' + str(row) + \
			   ' has a sigma (' + str(sig) + ') greater than 1.0 ns\n'
	return sigError

def GetMaximum(hist):
	maxBin = hist.GetMaximumBin()
	maximum = hist.GetBinCenter(maxBin)
	return maximum


if __name__ == "__main__":
	main()

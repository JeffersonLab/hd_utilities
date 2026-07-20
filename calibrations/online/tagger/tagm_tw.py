#! /usr/bin/python

# Python script to fit the timewalk curve for the TAGM
# Usage: python tw.py -b <filename> <run number>
# Created 5/3/16 barnes

from ROOT import *
import os,sys

def main():
	# Check for proper usage
	if (len(sys.argv) != 4):
		print 'Usage: python tw.py -b <filename> <run number>'
		return

	# Use command line arguments
	filename = sys.argv[2]
	run = int(sys.argv[3])

	# Check if the run used the old or new bias voltage scheme
	if (run < 11572) or (run > 30299):
		newV = False
	else:
		newV = True

	# Open input and output ROOT files
	rootfile = TFile.Open(str(filename))
	outfile = TFile.Open("results.root","recreate")
	outfile.cd()

	# Get offsets from first calibration step
	offsets = []
	offsets_ind = []
	offset_file = open('offsets-' + str(run) + '.txt', 'r')
	for line in offset_file:
		if int(line.split()[0]) == 0:
			offsets.append( float(line.split()[2]) )
		else:
			offsets_ind.append( float(line.split()[2]) )

	# If the histogram is empty use the summed output hist instead
	base = "TAGM_TW/tdc-rf/h_dt_vs_pp_tdc_"
	for i in range(1,103):
		# Summed outputs
		h = rootfile.Get(base+str(i))
		h.Write()
		p = tw_corr(h,0,i,newV, offsets, offsets_ind, run)
                if p is not None:
                        p.Write()

	outfile.Close()

def tw_corr(h,row,col,newV, offsets, offsets_ind, run):
	if not (col % 10):
		print('Calibrating column ' + str(col))
	# Create list of columns with individual readout
	indCol = [9,27,81,99]
	# Open files for writing constants
	if (row == 0 and col == 1):
		file1 = open('tw-corr-' + str(run) + '.txt','w')
	else:
		file1 = open('tw-corr-' + str(run) + '.txt','a')

	# shift histogram time axis based on first step calibration results
	xbins = h.GetXaxis().GetNbins()
	hnew = h.Clone()
	hnew.Reset()
        try:
                dtmean = GetMean(h)
        except:
                # if there's no data, bail
		c0 = 1
		c1 = -1
		c2 = 0
		c3 = 0
		dtmean = 0

                # Write constants to file
                file1.write(str(row) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
                            str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
                if col in indCol:
                        for j in range(1,6):
                                file1.write(str(j) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
                                            str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
                file1.close()
                return None

	ymax = h.GetYaxis().FindBin(dtmean + 15.0)
	ymin = h.GetYaxis().FindBin(dtmean - 5.0)
	for i in range(1,xbins+1):
		#for j in range(1,ybins+1):
		for j in range(ymin,ymax+1):
			x = hnew.GetXaxis().GetBinCenter(i)
			y = hnew.GetYaxis().GetBinCenter(j)
			y -= offsets[col - 1]
			n = int(h.GetBinContent(i, j))
			for k in range(n):
				hnew.Fill(x, y)

	# Find the reference time difference
        try:
                dtmean = GetMean(hnew)
        except:
                # if there's no data, bail
		c0 = 1
		c1 = -1
		c2 = 0
		c3 = 0
		dtmean = 0

                # Write constants to file
                file1.write(str(row) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
                            str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
                if col in indCol:
                        for j in range(1,6):
                                file1.write(str(j) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
                                            str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
                file1.close()
                return None

	ymax = h.GetYaxis().FindBin(dtmean + 15.0)
	ymin = h.GetYaxis().FindBin(dtmean - 5.0)

	# For low amplitude channels, remove tail beyond 3ns
	# This provides a better Profile for fitting the timewalk
	for i in range(1, xbins+1):
		#for j in range(1, ybins+1):
		for j in range(ymin, ymax+1):
			y = hnew.GetYaxis().GetBinCenter(j)
			if (y - dtmean > 3.0):
				hnew.SetBinContent(i,j,0)
	
	# Make timewalk fit function and apply to hist
	# New voltage scheme has larger pulse height, adjust the range if needed
	try:
		if (newV):
			f1 = TF1("f1","[0]+[1]*(1/(x+[3]) )**[2]",400,2000)
		else:
			#f1 = TF1("f1","[0]+[1]*(1/(x+[3]) )**[2]",125,2000) # runs before 30000
			f1 = TF1("f1","[0]+[1]*(1/(x+[3]) )**[2]",100,2000)
		f1.SetParameter(0,-1)
		f1.SetParameter(1,100)
		f1.SetParameter(2,0.7)
		f1.SetParameter(3,-90)
		f1.SetParName(0,"c0")
		f1.SetParName(1,"c1")
		f1.SetParName(2,"c2")
		f1.SetParName(3,"c3")

		#hnew.RebinX(4)
		hnew.GetYaxis().SetRangeUser(dtmean-5.0, dtmean+15.0)
		p = hnew.ProfileX()
		fitResult = p.Fit("f1","sRWq")

		c0 = fitResult.Parameters()[0]
		c1 = fitResult.Parameters()[1]
		c2 = fitResult.Parameters()[2]
		c3 = fitResult.Parameters()[3]

		h_adj = hnew.Clone()
		h_adj.Reset()

		for i in range(1,xbins+1):
			#for j in range(1,ybins+1):
			for j in range(ymin,ymax+1):
				x = hnew.GetXaxis().GetBinCenter(i)
				y = hnew.GetYaxis().GetBinCenter(j)
				y = f1.Eval(x) - y
				n = int(hnew.GetBinContent(i, j))
				for k in range(n):
					h_adj.Fill(x, y)
		dtmean = GetMean(h_adj)
	except:
		c0 = 1
		c1 = -1
		c2 = 0
		c3 = 0
		dtmean = 0

	# Write constants to file
	file1.write(str(row) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
                    str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
	if col in indCol:
		for j in range(1,6):
			file1.write(str(j) + '   ' + str(col) + '   ' + str(c0) + '   ' + str(c1) + '   ' +
        		            str(c2) + '   ' + str(c3) + '   ' + str(dtmean) + '\n')
	file1.close()

	return p
	#return h_adj

def GetMean(hist):
	py = hist.ProjectionY()
	ymax = py.GetBinCenter( py.GetMaximumBin() )
	fit = py.Fit("gaus","sq", "", ymax - 0.5, ymax + 0.5)
	dtmean = fit.Parameters()[1]
	return dtmean


if __name__ == "__main__":
	main()

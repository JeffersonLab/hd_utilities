RUN=$1

# script for writing configuration files for DAQ for DAC threshold of 50, 100, 200
# provide run number of threshold scan as input

# read ROOT tree of threshold scan data from online farm and fill histograms
cd ThresholdScan/
root -l -b -q writeScanHist.C\($RUN\)
mv scanHist.root scanHist_$RUN.root
cd ../

# nominal equalized gains from initial laser tests
root -l -b -q writeChannels.C\($RUN,50\)
root -l -b -q writeChannels.C\($RUN,100\)
root -l -b -q writeChannels.C\($RUN,200\)

# unity gain for all channels
root -l -b -q writeChannels.C\($RUN,50,1\)
root -l -b -q writeChannels.C\($RUN,100,1\)
root -l -b -q writeChannels.C\($RUN,200,1\)

# minimum unity gain for all channels
root -l -b -q writeChannels.C\($RUN,50,0,1\)
root -l -b -q writeChannels.C\($RUN,100,0,1\)
root -l -b -q writeChannels.C\($RUN,200,0,1\)

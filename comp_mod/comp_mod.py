#!/usr/bin/python

import sys
from xml.dom.minidom import parse
import xml.dom.minidom

# command line arguments
HIGHLIGHT = False  # Use ANSI escapes to highlight important lines (use -H to turn on)


INPUTFILE = "inputs.xml"
for arg in sys.argv[1:] :
	if arg.startswith('-'):
		if arg == '-H' : HIGHLIGHT = True
	else:
		INPUTFILE = arg


# constants
secondsPerDay = 24.*60.*60.
secondsPerYear = 365.25*secondsPerDay
secondsPerWeek = 7.0*secondsPerDay
secondsPerMonth = secondsPerYear/12.
weeksPerMonth = secondsPerMonth/secondsPerWeek
daysPerWeek = secondsPerWeek/secondsPerDay
daysPerMonth = secondsPerMonth/secondsPerDay
oneBillion = 1.0e9
floorDaysPerPacDay = 2

# expected units
UnitsExpected={}
UnitsExpected['triggerRate']        = 'Hz'
UnitsExpected['runningTimeOnFloor'] = 'days'
UnitsExpected['eventsize']          = 'kB'
UnitsExpected['eventsPerRun']       = 'Mevent'
UnitsExpected['reconstructionRate'] = 'Hz'
UnitsExpected['analysisRate'] = 'Hz'
UnitsExpected['incomingData']       = 'files'
UnitsExpected['calibRate']          = 'Mhr/week'
UnitsExpected['offlineMonitoring']  = 'Mhr/run'
UnitsExpected['simulationRate']     = 'Hz'

# input values (with unit checks
DOMTree = xml.dom.minidom.parse(INPUTFILE)
compMod = DOMTree.documentElement
parameters=compMod.getElementsByTagName("parameter")
for parameter in parameters:

	name  = parameter.getAttribute("name")
	value = parameter.getAttribute("value")
	units = parameter.getAttribute("units")
	if not parameter.hasAttribute("units"): units = None

	if name in UnitsExpected:
		if units != UnitsExpected[name]:
			sys.exit('%s in %s not in %s' % (name, units, UnitsExpected[name]))
	elif units != None:
		sys.exit('%s has units %s when none expected' % (name, units))

	# Set variable with attribute name
	exec(parameter.getAttribute("name") + '=' + parameter.getAttribute("value"))


# calculations
runningTimePac_days = runningTimeOnFloor/floorDaysPerPacDay
runningTimePac_weeks = runningTimePac_days/daysPerWeek
runningTimeOnFloor_s = runningTimeOnFloor*secondsPerDay

numberEvents = triggerRate*runningEfficiency*runningTimeOnFloor_s
numberProductionEvents = numberEvents*goodRunFraction
numberProductionEvents_billions = numberProductionEvents/oneBillion
reconstructionTimePerEvent_ms = 1000.0/reconstructionRate
reconstructionTime_s = reconPasses*numberProductionEvents/reconstructionRate
reconstructionTime_years = reconstructionTime_s/secondsPerYear
reconstructionTimeAllCores_s = reconstructionTime_s/cores
reconstructionTimeAllCores_weeks = reconstructionTimeAllCores_s/secondsPerWeek
reconstructionTimeAllCores_Mhr = reconstructionTime_s/3600.0/1000000.0
eventsize_bytes = eventsize*1024/compressionFactor
rawDataVolume_PB = eventsize_bytes*numberEvents/1.0E15
rawDataRateCompressed_GBps = eventsize_bytes*triggerRate/1.0E9
rawDataRateUncompressed_GBps = rawDataRateCompressed_GBps*compressionFactor
rawDataOffsite1month_MBps = rawDataVolume_PB*1.0E9/secondsPerMonth
RESTfractionCompressed = RESTfraction*compressionFactor
RESTDataVolume_PB = rawDataVolume_PB*RESTfractionCompressed*reconPasses
AnalysisDataVolume_PB = rawDataVolume_PB*RESTfractionCompressed*analysisPasses # Analysis pass data volume comparable to REST data volume
simulationDataVolume_PB = rawDataVolume_PB*RESTfractionCompressed*simulationpasses*simulatedPerRawEvent
simulationTimeGeneration_Mhr = numberProductionEvents_billions*1.0E9*simulationpasses*simulatedPerRawEvent/simulationRate/3600.0/1.0E6
simulationTimeReconstruction_Mhr = simulationTimeGeneration_Mhr*simulationRate/reconstructionRate
simulationTimeTotal_Mhr = simulationTimeGeneration_Mhr + simulationTimeReconstruction_Mhr

analysisCPU_Mhr = analysisPasses*numberProductionEvents/analysisRate/3600.0/1.0E6
calibCPU_Mhr = calibRate*runningTimeOnFloor/daysPerWeek
numberRuns = numberProductionEvents/(eventsPerRun*1.0E6)
offlineMonitoring_Mhr = offlineMonitoring*numberRuns
cpuRun_Mhr = eventsPerRun*1.0E6/reconstructionRate/3600.0/1.0E6
miscUserStudies_Mhr = miscUserStudies*cpuRun_Mhr
eventsPerFile = 20.0E9/(eventsize*1.0E3)
cpuFile_Mhr = eventsPerFile/reconstructionRate/3600.0/1.0E6
incomingData_Mhr = incomingData*numberRuns*cpuFile_Mhr
TOTAL_CPU_REAL_DATA = reconstructionTimeAllCores_Mhr + analysisCPU_Mhr + calibCPU_Mhr + offlineMonitoring_Mhr + miscUserStudies_Mhr + incomingData_Mhr

TOTAL_CPU_Mhr = TOTAL_CPU_REAL_DATA + simulationTimeTotal_Mhr
TOTAL_TAPE_PB = rawDataVolume_PB + RESTDataVolume_PB + AnalysisDataVolume_PB + simulationDataVolume_PB

# This is just for pretty printing below
compressed_str = '(compressed)'
uncompressed_str = '(uncompressed)'
if compressionFactor == 1 :
	compressed_str = ''
	uncompressed_str =''

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

BD = ''
if HIGHLIGHT : BD = bcolors.BOLD + bcolors.FAIL

# Print report
print    ''
print    '               GlueX Computing Model'
print    ' '*(25 - len(INPUTFILE)/2) + INPUTFILE
print    '=============================================='
print    '                 PAC Time: ' + '%3.1f' % runningTimePac_weeks + ' weeks'
print    '             Running Time: ' + '%3.1f' % (runningTimeOnFloor/7.0) + ' weeks'
print    '       Running Efficiency: ' + str(int(runningEfficiency*100.0)) + '%'
print    '  --------------------------------------'
print    '             Trigger Rate: ' + str(triggerRate/1000.0) + ' kHz'
print    '     Raw Data Num. Events: ' + '%3.1f' % numberProductionEvents_billions + ' billion (good production runs only)'
print    '     Raw Data compression: ' + '%3.2f' % compressionFactor
print    '      Raw Data Event Size: ' + str(eventsize) + ' kB ' + uncompressed_str
print    '  Front End Raw Data Rate: ' + '%3.2f' % rawDataRateUncompressed_GBps + ' GB/s ' + uncompressed_str
print    '       Disk Raw Data Rate: ' + '%3.2f' % rawDataRateCompressed_GBps + ' GB/s ' + compressed_str
print    '          Raw Data Volume: ' + '%3.3f' % rawDataVolume_PB + ' PB ' + compressed_str
print    '     Bandwidth to offsite: ' + '%3.0f' % rawDataOffsite1month_MBps + ' MB/s (all raw data in 1 month)'
print    '      REST/Raw size frac.: ' + '%3.2f' % (RESTfractionCompressed*100.0) + '%'
print    '         REST Data Volume: ' + '%3.3f' % RESTDataVolume_PB + ' PB (for ' + str(reconPasses) + ' passes)'
print    '     Analysis Data Volume: ' + '%3.3f' % AnalysisDataVolume_PB + ' PB (ROOT Trees for ' + str(analysisPasses) + ' passes)'
print BD+'   Total Real Data Volume: ' + '%3.1f' % (rawDataVolume_PB + RESTDataVolume_PB + AnalysisDataVolume_PB) + ' PB' + bcolors.ENDC
print    '  --------------------------------------'
print    '        Recon. time/event: ' + '%3.0f' % reconstructionTimePerEvent_ms + ' ms (' + str(reconstructionRate) + ' Hz/core)'
print    '           Available CPUs: ' + str(cores) + ' cores (full)'
print    '          Time to process: ' + '%3.1f' % reconstructionTimeAllCores_weeks + ' weeks (all passes)'
print    '        Good run fraction: ' + str(goodRunFraction)
print    '   Number of recon passes: ' + str(reconPasses)
print    'Number of analysis passes: ' + str(analysisPasses)
print    '       Reconstruction CPU: ' + '%3.1f' % reconstructionTimeAllCores_Mhr + ' Mhr'
print    '             Analysis CPU: ' + '%3.3f' % analysisCPU_Mhr + ' Mhr'
print    '          Calibration CPU: ' + '%3.1f' % calibCPU_Mhr + ' Mhr'
print    '   Offline Monitoring CPU: ' + '%3.1f' % offlineMonitoring_Mhr + ' Mhr'
print    '            Misc User CPU: ' + '%3.1f' % miscUserStudies_Mhr + ' Mhr'
print    '        Incoming Data CPU: ' + '%3.3f' % incomingData_Mhr + ' Mhr'
print BD+'      Total Real Data CPU: ' + '%3.1f' % TOTAL_CPU_REAL_DATA + ' Mhr' + bcolors.ENDC
print    '  --------------------------------------'
print    '       MC generation Rate: ' + '%3.1f' % simulationRate + ' Hz/core'
print    '      MC Number of passes: ' + '%3.1f' % simulationpasses
print    '      MC events/raw event: ' + '%3.2f' % simulatedPerRawEvent
print BD+'           MC data volume: ' + '%3.3f' % simulationDataVolume_PB + ' PB  (REST only)' + bcolors.ENDC
print    '        MC Generation CPU: ' + '%3.1f' % simulationTimeGeneration_Mhr + ' Mhr'
print    '    MC Reconstruction CPU: ' + '%3.1f' % simulationTimeReconstruction_Mhr + ' Mhr'
print BD+'             Total MC CPU: ' + '%3.1f' % simulationTimeTotal_Mhr + ' Mhr' + bcolors.ENDC
print    '  ---------------------------------------'
print    '                   TOTALS:'
print BD+'                      CPU: ' + '%3.1f' % TOTAL_CPU_Mhr + ' Mhr' + bcolors.ENDC
print BD+'                     TAPE: ' + '%3.1f' % TOTAL_TAPE_PB + ' PB' + bcolors.ENDC
print    ''




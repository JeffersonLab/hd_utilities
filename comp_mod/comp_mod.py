#!/usr/bin/python

import sys
from xml.dom.minidom import parse
import xml.dom.minidom

# command line arguments

INPUTFILE = "inputs.xml"
if len(sys.argv) > 1 : INPUTFILE = sys.argv[1]


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
triggerRateUnitsExpected = 'Hz'
runningTimeOnFloorUnitsExpected = 'days'
runningEfficiencyUnitsExpected = 'dl'
timePeriodUnitsExpected = 'months'
reconstructionRateUnitsExpected = 'Hz'
coresUnitsExpected = 'dl'
passesUnitsExpected = 'dl'
eventsizeUnitsExpected = 'kB'
compressionFactorUnitsExpected = 'dl'
RESTfractionUnitsExpected = 'dl'
simulationRateUnitsExpected = 'Hz'
simulationRateUnitsExpected = 'Hz'
simulationpassesUnitsExpected = 'dl'
simulatedPerRawEventUnitsExpected = 'dl'

# inputs

DOMTree = xml.dom.minidom.parse(INPUTFILE)
compMod = DOMTree.documentElement
parameters=compMod.getElementsByTagName("parameter")
for parameter in parameters:
    exec(parameter.getAttribute("name") + '=' + parameter.getAttribute("value"))
    exec(parameter.getAttribute("name") + 'Units = "' + parameter.getAttribute("units") + '"')
    name = parameter.getAttribute("name")
    units = parameter.getAttribute("units")
    value = parameter.getAttribute("value")
    unitsExpected = eval(parameter.getAttribute("name") + 'UnitsExpected')
    codeCheckUnits = "if %sUnitsExpected == '%s':\n %s_%s = %s\nelse:\n sys.exit('%s in %s not in %s')" % (name, units, name, units, value, name, units, unitsExpected)
    exec(codeCheckUnits)

# calculations

runningTimePac_days = runningTimeOnFloor_days/floorDaysPerPacDay
runningTimePac_weeks = runningTimePac_days/daysPerWeek
runningTimeOnFloor_s = runningTimeOnFloor_days*secondsPerDay
numberEvents_dl = triggerRate_Hz*runningEfficiency_dl*runningTimeOnFloor_s
numberEvents_billions = numberEvents_dl/oneBillion
timePeriod_s = timePeriod_months*secondsPerMonth
averageEventRate_Hz = numberEvents_dl/timePeriod_s
reconstructionTime_s = passes*numberEvents_dl/reconstructionRate_Hz
reconstructionTime_years = reconstructionTime_s/secondsPerYear
reconstructionTimeAllCores_s = reconstructionTime_s/cores
reconstructionTimeAllCores_weeks = reconstructionTimeAllCores_s/secondsPerWeek
reconstructionTimeAllCores_Mhr = reconstructionTime_s/3600.0/1000000.0
eventsize_bytes = eventsize*1024/compressionFactor
rawDataVolume_PB = eventsize_bytes*numberEvents_dl/1.0E15
rawDataRateCompressed_GBps = eventsize_bytes*triggerRate_Hz/1.0E9
rawDataRateUncompressed_GBps = rawDataRateCompressed_GBps*compressionFactor
rawDataOffsite1month_MBps = rawDataVolume_PB*1.0E9/secondsPerMonth
RESTfractionCompressed = RESTfraction*compressionFactor
RESTDataVolume_PB = rawDataVolume_PB*RESTfractionCompressed*passes
simulationDataVolume_PB = rawDataVolume_PB*RESTfractionCompressed*simulationpasses*simulatedPerRawEvent
simulationTimeGeneration_Mhr = numberEvents_billions*1.0E9*simulationpasses*simulatedPerRawEvent/simulationRate/3600.0/1.0E6
simulationTimeReconstruction_Mhr = simulationTimeGeneration_Mhr*simulationRate/reconstructionRate_Hz
simulationTimeTotal_Mhr = simulationTimeGeneration_Mhr + simulationTimeReconstruction_Mhr

TOTAL_CPU_Mhr = reconstructionTimeAllCores_Mhr + simulationTimeTotal_Mhr
TOTAL_TAPE_PB = rawDataVolume_PB + RESTDataVolume_PB + simulationDataVolume_PB

# This is just for pretty printing below
compressed_str = '(compressed)'
uncompressed_str = '(uncompressed)'
if compressionFactor == 1 :
	compressed_str = ''
	uncompressed_str =''

print ''
print '           GlueX Computing Model'
print ' '*(21 - len(INPUTFILE)/2) + INPUTFILE
print '=========================================='
print '         Trigger Rate: ' + str(triggerRate_Hz/1000.0) + ' kHz'
print '             PAC Time: ' + '%3.1f' % runningTimePac_weeks + ' weeks'
print '         Running Time: ' + '%3.1f' % (runningTimeOnFloor_days/7.0) + ' weeks'
print '   Running Efficiency: ' + str(int(runningEfficiency_dl*100.0)) + '%'
print '  --------------------------------------'
print '  Reconstruction Rate: ' + str(reconstructionRate_Hz) + ' Hz/core'
print '       Available CPUs: ' + str(cores) + ' cores (full)'
print '      Time to process: ' + '%3.1f' % reconstructionTimeAllCores_weeks + ' weeks (all passes)'
print '     Number of passes: ' + str(passes)
print '   Reconstruction CPU: ' + '%3.1f' % reconstructionTimeAllCores_Mhr + ' Mcore-hr'
print '  --------------------------------------'
print ' Raw Data Num. Events: ' + '%5.1f' % numberEvents_billions + ' billion'
print ' Raw Data compression: ' + '%3.2f' % compressionFactor
print '  Raw Data Event Size: ' + str(eventsize) + ' kB ' + compressed_str
print '    Max Raw Data Rate: ' + '%3.2f' % rawDataRateUncompressed_GBps + ' GB/s ' + uncompressed_str
print '      Raw Data Volume: ' + '%3.3f' % rawDataVolume_PB + ' PB ' + compressed_str
print ' Bandwidth to offsite: ' + '%3.0f' % rawDataOffsite1month_MBps + ' MB/s (all raw data in 1 month)'
print '  REST/Raw size frac.: ' + '%3.2f' % (RESTfractionCompressed*100.0) + '%'
print '     REST Data Volume: ' + '%3.3f' % RESTDataVolume_PB + ' PB (for ' + str(passes) + ' passes)'
print '  --------------------------------------'
print '   MC generation Rate: ' + '%3.1f' % simulationRate + ' Hz/core'
print '  MC Number of passes: ' + '%3.1f' % simulationpasses
print '  MC events/raw event: ' + '%3.2f' % simulatedPerRawEvent
print '       MC data volume: ' + '%3.3f' % simulationDataVolume_PB + ' PB  (REST only)'
print '    MC Generation CPU: ' + '%3.1f' % simulationTimeGeneration_Mhr + ' Mcore-hr'
print 'MC Reconstruction CPU: ' + '%3.1f' % simulationTimeReconstruction_Mhr + ' Mcore-hr'
print '               MC CPU: ' + '%3.1f' % simulationTimeTotal_Mhr + ' Mcore-hr'
print '  --------------------------------------'
print '               TOTALS:'
print '                  CPU: ' + '%3.1f' % TOTAL_CPU_Mhr + ' Mcore-hr'
print '                 TAPE: ' + '%3.1f' % TOTAL_TAPE_PB + ' PB'
print ''




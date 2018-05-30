#!/usr/bin/python

import sys
from xml.dom.minidom import parse
import xml.dom.minidom

# constants

secondsPerDay = 24.*60.*60.
secondsPerYear = 365.25*secondsPerDay
secondsPerWeek = 7.0*secondsPerDay
secondsPerMonth = secondsPerYear/12.
weeksPerMonth = secondsPerMonth/secondsPerWeek
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
RESTfractionUnitsExpected = 'dl'
simulationRateUnitsExpected = 'Hz'
simulationRateUnitsExpected = 'Hz'
simulationpassesUnitsExpected = 'dl'
simulatedPerRawEventUnitsExpected = 'dl'

# inputs

DOMTree = xml.dom.minidom.parse("inputs.xml")
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

pacRunningTime_days = runningTimeOnFloor_days/floorDaysPerPacDay
runningTimeOnFloor_s = runningTimeOnFloor_days*secondsPerDay
numberEvents_dl = triggerRate_Hz*runningEfficiency_dl*runningTimeOnFloor_s
numberEvents_billions = numberEvents_dl/oneBillion
timePeriod_s = timePeriod_months*secondsPerMonth
averageEventRate_Hz = numberEvents_dl/timePeriod_s
reconstructionTime_s = numberEvents_dl/reconstructionRate_Hz
reconstructionTime_years = reconstructionTime_s/secondsPerYear
reconstructionTimeAllCores_s = reconstructionTime_s/cores
reconstructionTimeAllCores_weeks = reconstructionTimeAllCores_s/secondsPerWeek
reconstructionTimeAllCores_Mhr = reconstructionTime_s/3600.0/1000000.0
eventsize_bytes = eventsize*1024
rawDataVolume_PB = eventsize_bytes*triggerRate_Hz*runningTimeOnFloor_s/1.0E15
rawDataRate_GBps = eventsize_bytes*triggerRate_Hz/1.0E9
RESTDataVolume_PB = rawDataVolume_PB*RESTfraction*passes
simulationDataVolume_PB = rawDataVolume_PB*RESTfraction*simulationpasses*simulatedPerRawEvent
simulationTimeGeneration_Mhr = numberEvents_billions*1.0E9*simulationpasses*simulatedPerRawEvent/simulationRate/3600.0/1.0E6
simulationTimeReconstruction_Mhr = simulationTimeGeneration_Mhr*simulationRate/reconstructionRate_Hz
simulationTimeTotal_Mhr = simulationTimeGeneration_Mhr + simulationTimeReconstruction_Mhr

TOTAL_CPU_Mhr = reconstructionTimeAllCores_Mhr + simulationTimeTotal_Mhr
TOTAL_TAPE_PB = rawDataVolume_PB + RESTDataVolume_PB + simulationDataVolume_PB

print ''
print '          GlueX Computing Model'
print '=========================================='
print '         Trigger Rate: ' + str(triggerRate_Hz/1000.0) + ' kHz'
print '         Running Time: ' + str(runningTimeOnFloor_days/7.0) + ' weeks'
print '   Running Efficiency: ' + str(int(runningEfficiency_dl*100.0)) + '%'
print '  --------------------------------------'
print '  Reconstruction Rate: ' + str(reconstructionRate_Hz) + ' Hz/core'
print '       Available CPUs: ' + str(cores) + ' cores (full)'
print '      Time to process: ' + '%3.1f' % reconstructionTimeAllCores_weeks + ' weeks/pass'
print '     Number of passes: ' + str(passes)
print '   Reconstruction CPU: ' + '%3.1f' % reconstructionTimeAllCores_Mhr + ' Mcore-hr'
print '  --------------------------------------'
print '  Raw Data Event Size: ' + str(eventsize) + ' kB'
print '        Raw Data Rate: ' + '%3.2f' % rawDataRate_GBps + ' GB/s'
print '      Raw Data Volume: ' + '%3.1f' % rawDataVolume_PB + ' PB'
print '  REST/Raw size frac.: ' + str(int(RESTfraction*100.0)) + '%'
print '     REST Data Volume: ' + '%3.1f' % RESTDataVolume_PB + ' PB (for ' + str(passes) + ' passes)'
print '  --------------------------------------'
print '   MC generation Rate: ' + '%3.1f' % simulationRate + ' Hz/core'
print '  MC Number of passes: ' + '%3.1f' % simulationpasses
print '  MC events/raw event: ' + '%3.1f' % simulatedPerRawEvent
print '       MC data volume: ' + '%3.1f' % simulationDataVolume_PB + ' PB  (REST only)'
print '    MC Generation CPU: ' + '%3.1f' % simulationTimeGeneration_Mhr + ' Mcore-hr'
print 'MC Reconstruction CPU: ' + '%3.1f' % simulationTimeReconstruction_Mhr + ' Mcore-hr'
print '               MC CPU: ' + '%3.1f' % simulationTimeTotal_Mhr + ' Mcore-hr'
print '  --------------------------------------'
print '               TOTALS:'
print '                  CPU: ' + '%3.1f' % TOTAL_CPU_Mhr + ' Mcore-hr'
print '                 TAPE: ' + '%3.1f' % TOTAL_TAPE_PB + ' PB'
print ''




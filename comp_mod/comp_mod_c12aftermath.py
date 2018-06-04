#!/usr/bin/python

import sys
from xml.dom.minidom import parse
import xml.dom.minidom

# constants

secondsPerHour = 60*60
secondsPerDay = 24*secondsPerHour
secondsPerYear = 365.25*secondsPerDay
secondsPerWeek = 7.0*secondsPerDay
secondsPerMonth = secondsPerYear/12.
weeksPerMonth = secondsPerMonth/secondsPerWeek
daysPerMonth = secondsPerMonth/secondsPerDay
oneBillion = 1.0e9
oneMillion = 1.0e6
HertzPerKiloHertz = 1000

# expected units
jobTimeUnitsExpected = 'hours'
eventsPerFileUnitsExpected = 'millions'
coresPerJobUnitsExpected = 'dl'
triggerRateUnitsExpected = 'kHz'
runningTimeUnitsExpected = 'weeks'
runningEfficiencyUnitsExpected = 'dl'
coresUnitsExpected = 'dl'

# inputs

DOMTree = xml.dom.minidom.parse("inputs_c12aftermath.xml")
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

triggerRate_Hz = triggerRate_kHz*HertzPerKiloHertz
runningTime_s = runningTime_weeks*secondsPerWeek
jobTime_s = jobTime_hours*secondsPerHour
eventsPerFile_dl = eventsPerFile_millions*oneMillion
totalNumberOfEvents_dl = runningTime_s*triggerRate_Hz*runningEfficiency_dl
reconstructionTimePerEventOneCore_s = jobTime_s/eventsPerFile_dl*coresPerJob_dl
reconstructionRateOneCore_Hz = 1/reconstructionTimePerEventOneCore_s
totalReconstructionTimeOneCore_s = totalNumberOfEvents_dl*reconstructionTimePerEventOneCore_s
totalReconstructionTimeAllCores_s = totalReconstructionTimeOneCore_s/cores
totalReconstructionTimeAllCores_hours = totalReconstructionTimeAllCores_s/secondsPerHour
totalReconstructionTimeAllCores_weeks = totalReconstructionTimeAllCores_s/secondsPerWeek
print(triggerRate, triggerRateUnits, runningTime, runningTimeUnits, reconstructionRateOneCore_Hz, 'Hz', totalReconstructionTimeAllCores_hours, 'hours', totalReconstructionTimeAllCores_weeks, 'weeks')
def output(name, units):
  statement = "print('" + name + " = %s " + units + "' % " + name + "_" + units + ")"
  print(statement)
  exec(statement)
output('totalReconstructionTimeAllCores', 'weeks')


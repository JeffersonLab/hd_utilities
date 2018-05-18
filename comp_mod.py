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

# expected units
triggerRateUnitsExpected = 'Hz'
runningTimeUnitsExpected = 'days'
runningEfficiencyUnitsExpected = 'dl'
timePeriodUnitsExpected = 'months'
reconstructionRateUnitsExpected = 'Hz'
coresUnitsExpected = 'dl'

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

runningTime_s = runningTime_days*secondsPerDay
numberEvents_dl = triggerRate_Hz*runningEfficiency_dl*runningTime_s
numberEvents_billions = numberEvents_dl/oneBillion
timePeriod_s = timePeriod_months*secondsPerMonth
averageEventRate_Hz = numberEvents_dl/timePeriod_s
reconstructionTime_s = numberEvents_dl/reconstructionRate_Hz
reconstructionTime_years = reconstructionTime_s/secondsPerYear
reconstructionTimeAllCores_s = reconstructionTime_s/cores
reconstructionTimeAllCores_weeks = reconstructionTimeAllCores_s/secondsPerWeek
print(triggerRate, triggerRateUnits, runningTime, runningTimeUnits, timePeriod, timePeriodUnits, numberEvents_billions, 'billion', averageEventRate_Hz, 'Hz', reconstructionTime_years, 'years', reconstructionTimeAllCores_weeks, 'weeks')

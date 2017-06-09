#!/usr/bin/python

from xml.dom.minidom import parse
import xml.dom.minidom

# constants

secondsPerDay = 24.*60.*60.
secondsPerYear = 365.25*secondsPerDay
secondsPerWeek = 7.0*secondsPerDay
secondsPerMonth = secondsPerYear/12.
weeksPerMonth = secondsPerMonth/secondsPerWeek
daysPerMonth = secondsPerMonth/secondsPerDay

# inputs

DOMTree = xml.dom.minidom.parse("inputs.xml")
compMod = DOMTree.documentElement
parameters=compMod.getElementsByTagName("parameter")
for parameter in parameters:
    print parameter
    print "param name = %s" % parameter.getAttribute("name")
    exec(parameter.getAttribute("name") + '=' + parameter.getAttribute("value"))
    exec(parameter.getAttribute("name") + 'Units = "' + parameter.getAttribute("units") + '"')

# calculations

averageEventRateRaw = triggerRate*runningEfficiency*(runningTime/timePeriod)
averageEventRate = averageEventRateRaw/daysPerMonth
averageEventRateUnits = "Hz"
print(triggerRate, triggerRateUnits, runningTime, runningTimeUnits, timePeriod, timePeriodUnits, averageEventRate, averageEventRateUnits)

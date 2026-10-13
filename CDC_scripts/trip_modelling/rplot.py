
import sys
from ROOT import TGraph, TCanvas
from array import array

if len(sys.argv) < 2:
    exit('Usage: plot.py <straw_number> [show-plot]')

showplot = False
if len(sys.argv) == 3:
    showplot = True
    
straw = sys.argv[1]

filename = 'straw_eff_per_run/eff_' + straw

x = array('d')
y =array('d')

# 1. Open and parse the text file line by line
with open(filename, 'r') as file:
    for line in file:
        # Split line into separate values (defaults to splitting by whitespace)
        values = line.split()
        if len(values) >= 2:
            x.append(float(values[0]))  # Convert X value to number
            y.append(float(values[1]))  # Convert Y value to number

if len(x) == 0:
    exit('data file empty')

c = TCanvas("c","Eff",900,500)   
g = TGraph( len(x), x, y )
g.SetName("g")
g.SetTitle("Straw "+straw)
g.GetXaxis().SetNoExponent(True)
g.GetXaxis().SetTitle( 'Run number' )
g.GetYaxis().SetTitle( 'Efficiency' )
g.SetMarkerStyle( 20 )
g.SetMarkerSize( 0.7 )
g.SetMarkerColor(9)


g.Draw("ap")
c.SetGrid()
c.SaveAs("straw_eff_per_run/n"+straw+".png")

if showplot:
    input()

'''
plt.scatter(x, y, color='red')
plt.title(filename)
plt.xlabel('Run')
plt.ylabel('Efficiency')
plt.show()


lastvalue = ""
range_start = int(x[0])

for i in range(0, len(x)):

    eff = 1.03*y[i]   # max eff is 97%

    roundedeff = f"{round(20*eff)/20:.2f}"

    if i == 0:
        lastvalue = roundedeff        
        continue

    if roundedeff != lastvalue:
        range_end = int(x[i-1])
        print(f"{range_start} - {range_end} {roundedeff}")
        lastvalue = roundedeff
        range_start = int(x[i])

range_end = int(x[len(x)-1])
print(f"{range_start} - {range_end} {roundedeff}")
'''

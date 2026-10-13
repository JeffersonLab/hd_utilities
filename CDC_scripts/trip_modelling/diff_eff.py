import os
import sys

eff1 = []
eff2 = []

with open("eff_150097_150102.txt","r") as f:
    eff1 = [float(line.strip()) for line in f]

#with open("eff_150251_150255.txt","r") as f:
with open("eff_150181_150184.txt","r") as f:    
    eff2 = [float(line.strip()) for line in f]    


diff = []
bigdiff = []
for x in range(0,3522):
    thisdiff = eff2[x] - eff1[x]
    diff.append(thisdiff)
    if abs(thisdiff) >= 0.05 :
        bigdiff.append(thisdiff)
        print('%i %.2f %.2f %.2f'%(x+1, eff1[x], eff2[x], thisdiff))
    else:
        bigdiff.append(0)


    

     


import sys

if len(sys.argv) < 3:
    exit('Usage: python round_efficiency.py <filename> <max>\n eg python round_efficiency.py straw_eff_per_run/eff_97.txt 0.97\n')

filename = sys.argv[1]
scale = float(sys.argv[2])



# for straw 97 max efficiency is 97% scale up x 1/0.97 = 1.03
# straw 2460 max efficiency is 94%, scale up by 1.064


x = []
y = []

# 1. Open and parse the text file line by line
with open(filename, 'r') as file:
    for line in file:
        # Split line into separate values (defaults to splitting by whitespace)
        values = line.split()
        if len(values) >= 2:
            x.append(float(values[0]))  # Convert X value to number
            y.append(float(values[1]))  # Convert Y value to number

reff = []

for i in range(0, len(x)):

    eff = y[i]/scale   # max eff is 97%

    if eff <= 0.5:
        eff = 0
    
    reff.append( f"{round(20*eff)/20:.2f}")



# no special action needed if reff = '1.00'

for target in ['0.95', '0.90', '0.85', '0.80', '0.75', '0.70', '0.65', '0.60', '0.55', '0.00' ]:
    result = [int(x) for x, reff in zip(x, reff) if reff == target]
    #print(target, result, '\n')

    arrayname = int(100*float(target))
    
    if len(result) > 0:
        print()
        print('_'+str(arrayname) +' = ', result)


    

'''
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

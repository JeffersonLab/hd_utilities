
# script to deal with data from accelerator BTA log entries
# don't entirely understand this calculation
from __future__ import print_function
import sys,os


day_bta = []
swing_bta = []
owl_bta = []

# add data
day_bta.append( (6.17, 0.32, 0.63, 0.88) )
swing_bta.append( (6.89, 0.08, 0.32, 0.71) )
owl_bta.append( (6.87, 0.22, 0.11, 0.8) )

day_bta.append( (0.77, 0.02, 0, 7.21) )
swing_bta.append( (4.36, 0.39, 0.75, 2.5) ) 
owl_bta.append( (6.46, 0.37, 0.08, 1.09) )

day_bta.append( (5.59, 1.2, 0.03, 1.18) )
swing_bta.append( (5.82, 0.35, 1.12, 0.71) )
owl_bta.append( (7, 0.29, 0.07, 0.64) )

day_bta.append( (6.5, 0.44, 0.32, 0.74) )
swing_bta.append( (7.24, 0.07, 0.17, 0.52) )
owl_bta.append( (6.32, 0.46, 0.03, 1.19) )

day_bta.append( (5.68, 0.23, 1.19, 0.9) )
swing_bta.append( (6.92, 0.33, 0.09, 0.66) )
owl_bta.append( (6.77, 0.23, 0.13, 0.87) )

day_bta.append( (1.05, 0.11, 0.01, 6.8) )
swing_bta.append( (0.62, 0, 0.38, 7) )
owl_bta.append( (2.14, 0.44, 0, 5.42) )

day_bta.append( (1.71, 0.07, 0, 6.22) ) 
swing_bta.append( (1.65, 0.08, 0.03, 6.21) )
owl_bta.append( (5.36, 0.93, 0.31, 1.4) )




# print things out
DAY = int(sys.argv[1])-1

TOTAL_ABU = day_bta[DAY][0] + day_bta[DAY][1] + swing_bta[DAY][0] + swing_bta[DAY][1] + owl_bta[DAY][0] + owl_bta[DAY][1]
#TOTAL_BNA = day_bta[DAY][2] + swing_bta[DAY][2] + owl_bta[DAY][2]
#TOTAL_BANU = day_bta[DAY][3] + swing_bta[DAY][3] + owl_bta[DAY][3]
TOTAL_BNA = day_bta[DAY][3] + swing_bta[DAY][3] + owl_bta[DAY][3]
TOTAL_BANU = day_bta[DAY][2] + swing_bta[DAY][2] + owl_bta[DAY][2]

print("{|border",
      "|-",
      "! Shift  !! ABU !! BNA !! BANU ",
      "|-",
      "|Day   || %4.2f || %4.2f || %4.2f"%(day_bta[DAY][0] + day_bta[DAY][1],day_bta[DAY][3],day_bta[DAY][2]),
      "|-",
      "|Swing || %4.2f || %4.2f || %4.2f"%(swing_bta[DAY][0] + swing_bta[DAY][1],swing_bta[DAY][3],swing_bta[DAY][2]),
      "|-",
      "|Owl   || %4.2f || %4.2f || %4.2f"%(owl_bta[DAY][0] + owl_bta[DAY][1],owl_bta[DAY][3],owl_bta[DAY][2]),
      "|-",
      "|Total || %4.2f || %4.2f || %4.2f"%(TOTAL_ABU,TOTAL_BNA,TOTAL_BANU),
      "|-",
      "|}", sep='\n')

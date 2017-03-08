
# script to summarize BTA information from Hall D log entries
from __future__ import print_function
import sys,os


day_bta = []
swing_bta = []
owl_bta = []

# add data:  ABU BANU BNA ACC / ER PCC UED (first 4 accelerator, last 3 experiment)
# data comes from HDLOG BTA log entries
# Weds
day_bta.append( (335, 89, 56, 0, 386, 14, 80) )
swing_bta.append( (386.4667, 45.2833, 48.25, 0, 480, 0, 0) )
owl_bta.append( (403.35, 14.8833, 61.7667, 0, 470.6, 9.4, 0) )

# Thurs
day_bta.append( (5.7, 0, 474.3, 0, 480, 0, 0) )
swing_bta.append( (215, 125, 140, 0, 345, 90, 45) )
owl_bta.append( (365.1833, 13.7667, 101.05, 0, 469, 11, 0) )

# Fri
day_bta.append( (0, 0, 0, 0, 3850, 15, 80) )  # ??
swing_bta.append( (336.5, 100.5, 43, 0, 400, 60, 20) )
owl_bta.append( (406.5167, 9.4167, 64.0667, 0, 471, 9, 0) )

# Sat
day_bta.append( (363.7167, 52.0667, 64.2167, 0, 428, 0, 52) )
swing_bta.append( (416.55, 25.6833, 37.7667, 0, 453, 17, 10) )
owl_bta.append( (313.1333, 64.1333, 102.7333, 0, 375, 0, 105) )

# Sun
day_bta.append( (280.3167, 130.9667, 68.7167, 0, 349, 0, 131) )
swing_bta.append( (0, 0, 0, 0, 0, 0, 0) )
owl_bta.append( (385.6667, 13.9667, 80.3667, 0, 467, 13, 0) )

# Mon
day_bta.append( (0, 0, 0, 0, 0, 0, 0) )
swing_bta.append( (34.7667, 24.1833, 421.05, 0, 305, 0, 175) )
owl_bta.append( (161.6, 17.05, 301.35, 0, 465, 15, 0) )

# Tues
day_bta.append( (49.6333, 0, 430.3667, 0, 480, 0, 0) )
swing_bta.append( (145.8333, 9.1, 49.1833, 275.8833, 440, 5, 35) )
owl_bta.append( (264.6833, 59.6333, 155.6833, 0, 458, 22, 0) )


DAY = None
do_summary = False
# handle some command line arguments
if len(sys.argv) < 2:
    print("specify which day (number) or -summmary!")
    exit(0)

if sys.argv[1] == "-summary":
    do_summary = True
else:
    DAY = int(sys.argv[1])

# summarize
if do_summary:
    print( "         ABU   BANU   BNA   ACC     ER   PCC   UED")
    TOTAL = [0, 0, 0, 0, 0, 0, 0]
    DAYS = []
    for i in xrange(7):
        today = [0, 0, 0, 0, 0, 0, 0]
        for j in xrange(7):
            today[j] += day_bta[i][j]/60.
            today[j] += swing_bta[i][j]/60.
            today[j] += owl_bta[i][j]/60.
            
            TOTAL[j] += today[j]

        print("Day %d  %6.1f %5.1f %5.1f %5.1f   %5.1f %5.1f %5.1f"%(i,today[0],today[1],today[2],today[3],today[4],today[5],today[6]))
        print("")
        print("TOTAL  %6.1f %5.1f %5.1f %5.1f   %5.1f %5.1f %5.1f"%(TOTAL[0],TOTAL[1],TOTAL[2],TOTAL[3],TOTAL[4],TOTAL[5],TOTAL[6]))

else:
    TOTAL_ABU = day_bta[DAY][0] + swing_bta[DAY][0] + owl_bta[DAY][0]
    TOTAL_BANU = day_bta[DAY][1] + swing_bta[DAY][1] + owl_bta[DAY][1]
    TOTAL_BNA = day_bta[DAY][2] + swing_bta[DAY][2] + owl_bta[DAY][2]
    print("{|border",
          "|-",
          "! Shift  !! ABU !! BNA !! BANU ",
          "|-",
          "|Day   || %4.2f || %4.2f || %4.2f"%(day_bta[DAY][0]/60.,day_bta[DAY][1]/60.,day_bta[DAY][2]/60.),
          "|-",
          "|Swing || %4.2f || %4.2f || %4.2f"%(swing_bta[DAY][0]/60.,swing_bta[DAY][1]/60.,swing_bta[DAY][2]/60.),
          "|-",
          "|Owl   || %4.2f || %4.2f || %4.2f"%(owl_bta[DAY][0]/60.,owl_bta[DAY][1]/60.,owl_bta[DAY][2]/60.),
          "|-",
          "|Total || %4.2f || %4.2f || %4.2f"%(TOTAL_ABU/60.,TOTAL_BNA/60.,TOTAL_BANU/60.),
          "|-",
          "|}", sep='\n')


"""
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
"""

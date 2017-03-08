import LT.box as B
import numpy as np
import argparse as AG


parser = AG.ArgumentParser()
parser.add_argument("input_files", nargs = '*', help="list of input files ", default = 'BTM.data')
parser.add_argument('-s', '--style', type = int,  help="table style (0 = std, 1 = wiki)", default = 0)
parser.add_argument('-t', '--total', type = int,  help=" 1 = get total over several files", default = 0)
args = parser.parse_args()

files = args.input_files

ttype = args.style

shifts = ['Day', 'Swing', 'Owl']
ABU_TOTAL = 0.
BNA_TOTAL = 0.
BANU_TOTAL = 0.

Grand_TOTAL = 0.

for f in files:
    d = B.get_file(f)
    o = open(f + '.wiki', 'w')
    ABU_all =  np.array(d['ABU'])/60.
    BNA_all =  np.array(d['BNA'])/60.
    BANU_all = np.array(d['BANU'])/60.
    ABU_all_tot = ABU_all.sum()
    BNA_all_tot = BNA_all.sum()
    BANU_all_tot = BANU_all.sum()
    total_hours = ABU_all_tot + BNA_all_tot + BANU_all_tot

    n_split = int(ABU_all.shape[0]/8)
    
    ABU = np.array_split( ABU_all, n_split)
    BNA = np.array_split( BNA_all, n_split)
    BANU = np.array_split( BANU_all, n_split)
    
    print "------------------------------------------"
    print "Analyzing : ", f
    print "------------------------------------------"
    print " Shift    |  ABU   |  BNA  |  BANU   "
    print "------------------------------------------"
    if (ttype == 1):
        o.write('{|class="wikitable"\n|+Beam Time Summary \n|-\n|Shift\n |ABU\n |BNA\n |BANU\n|-\n') 
    else:
        o.write('{|border\n |-\n! Shift  !! ABU !! BNA !! BANU \n |-\n') 
    for i, abu in enumerate(ABU):
        current_shift = shifts[i]
        bna = BNA[i]
        banu = BANU[i]
        ABU_tot = abu.sum()
        BNA_tot = bna.sum()
        BANU_tot = banu.sum()
        sum_tot = ABU_tot + BNA_tot + BANU_tot
        print " {:5s}    |  {:5.2f} | {:5.2f} | {:5.2f}  ".format( current_shift, ABU_tot, BNA_tot, BANU_tot)
        if (ttype == 1):
            o.write('|{:5s}\n|{:5.2f}\n |{:5.2f}\n| {:5.2f}\n |-\n'.format( current_shift, ABU_tot, BNA_tot, BANU_tot))
        else:
            o.write('|{:5s} ||{:5.2f} ||{:5.2f} || {:5.2f}\n |-\n'.format( current_shift, ABU_tot, BNA_tot, BANU_tot))
    print     "------------------------------------------"
    print     " Total    |  {:5.2f} | {:5.2f} | {:5.2f}  ".format(ABU_all_tot, BNA_all_tot, BANU_all_tot)
    if (ttype == 1):
        o.write('|Total\n |{:5.2f}\n |{:5.2f}\n |{:5.2f}\n|-\n'.format(ABU_all_tot, BNA_all_tot, BANU_all_tot) + '|}\n')
    else:
        o.write('|Total ||{:5.2f} ||{:5.2f}  ||{:5.2f}\n|-\n'.format(ABU_all_tot, BNA_all_tot, BANU_all_tot) + '|}\n')
    print     "------------------------------------------"
    print "Grand Total = {:.2f}".format(total_hours)
    o.close()    
    if args.total > 0:
        ABU_TOTAL += ABU_all_tot 
        BNA_TOTAL += BNA_all_tot 
        BANU_TOTAL += BANU_all_tot 
        Grand_TOTAL += total_hours 
# all done
if args.total > 0:
    print     "------------------------------------------"
    print "{:5s}    |  {:5.2f} | {:5.2f} | {:5.2f}  ".format( "Final total" , ABU_TOTAL, BNA_TOTAL, BANU_TOTAL)
    print     "------------------------------------------"
    print "Grand TOTAL = {:.2f}".format(Grand_TOTAL)
# 

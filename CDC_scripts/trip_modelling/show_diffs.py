import sys
import os

tolerance = 0.1

def main():

    if len(sys.argv) != 3 :
        exit('Usage: python show_diffs.py <run1> <run2>\n')

    run1 = sys.argv[1]
    run2 = sys.argv[2]

    fname1 = get_filename(run1)
    fname2 = get_filename(run2)

    if not os.path.exists(fname1):
        exit('Cannot find file for run',run1)

    if not os.path.exists(fname2):
        exit('Cannot find file for run',run2)

    
    different = False
    
    with open(fname1, "r") as f1, open(fname2, "r") as f2:
        list1 = f1.readlines()
        list2 = f2.readlines()

        for i in range(0, len(list1)):
            n1 = float(list1[i])
            n2 = float(list2[i])

            diff = abs(n1 - n2)
            if diff >= tolerance:
                different = True
                print(i, n1, n2)
    



def get_filename(run):
    return "eff_trip_suppressed/eff_" + run + ".txt"



                   
if __name__ == "__main__":
    main()

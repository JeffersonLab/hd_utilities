import sys
import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet


def LoadCCDB():
    sqlite_connect_str = "mysql://ccdb_user@hallddb.jlab.org/ccdb"
    provider = ccdb.AlchemyProvider()           # this class has all CCDB manipulation functions
    provider.connect(sqlite_connect_str)        # use usual connection string to connect to database
    provider.authentication.current_user_name = "njarvis"  # to have a name in logs

    return provider


def main():

    if len(sys.argv) < 3:
      print "Usage: recalc_gain.py <run_number> <dedx>"
      print "eg recalc_gain.py 81555 1.9708 to calculate the new gcf"
      exit()

    run = int(sys.argv[1])
    dedx = float(sys.argv[2])

    # Load CCDB
    ccdb_conn = LoadCCDB()

    assignment = ccdb_conn.get_assignment("/CDC/digi_scales", run, "default")
    digi_scales = assignment.constant_set.data_table
    gcf = float(digi_scales[0][0])


    #dedx should be at 2.02

    newgain = 2.01992*gcf/dedx
    filename="newgain_"+str(run)+".txt"

    f = open(filename,"w")
    f.write("%.4f 0.8\n"%(newgain))    
    f.close()

    print "Run %d  old gcf %6.3f new gcf %.4f written to %s"%(run,gcf,newgain,filename)
    print "ccdb add -r %i-%i /CDC/digi_scales %s"%(run,run,filename)

## main function 
if __name__ == "__main__":
    main()

    

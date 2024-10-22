def find_level_change(line,linenum) :

    blist = ""
    for c in line:
        if c == "{" or c == "}" :
            blist = blist + c
            
    if blist == "" :
        return 0
              
    blist = blist.replace("{}","")
    blist = blist.replace("{}","")
    blist = blist.replace("{}","")
    blist = blist.replace("}{","")    
    
    # hopefully there won't be multiple { { or } } on same line            
    if blist == "" :
        return 0
    elif blist == "{" :
        return 1
    elif blist == "}" :
        return -1
    else :
        return("  Can't cope with syntax at line "+str(linenum))
        return 0
        

def check_this_file(fname) :

    f = open(fname, "r")

    linenum = 0
    level = 0          # depth inside {
    lock_level = -1    # depth when lock encountered
    f_locked = False   # fill lock state, set/unset when the lock/unlock code is encountered
    w_locked = False   # write lock state, set/unset when the lock/unlock code is encountered
    level_unlocked = 0  # set this when encountering an unlock at a different level
    warned = False      # so as not to repeat identical warnings

    f_lockstate = False    # used to remember the lock state when entering a code block
    w_lockstate = False

    cb_f_lockstate = False    # used to remember the lock state when meeting continue or break
    cb_w_lockstate = False

    cb_level = 0     # block level (depth of { ) when continue/break was met

    in_finit = False   # set true inside init and fini, which are not threaded

    printout=[]

    locks_used = False # set true if locks are used at all
    fills_used = False # set true if Fill commands are found

    
    for line in f:
    
        linenum = linenum+1

        #print(level, linenum,':',line)        

        # strip anything after comment
    
        x = line.split("//")[0]
        x = x.strip()

        dlevel = find_level_change(x,linenum)

        try:
            testforint = int(dlevel)
        except ValueError:
            printout.append(dlevel)
            dlevel = 0
        
        if dlevel > 0 :              # went into a new block
            f_lockstate = f_locked
            w_lockstate = w_locked
            #print(linenum,": level up, locks",f_locked,w_locked)

        if dlevel < 0 :
            if cb_level == level :   # there was a continue or break within previous block
                if (cb_f_lockstate != f_locked) or (cb_w_lockstate != w_locked) :
                    printout.append('Continue or break skipped the lock state change inside the code block ending at line'+str(linenum))
                cb_level = 0
            #print(linenum,": level down, locks",f_locked,w_locked)
        level = level + dlevel
        
#        if dlevel != 0 :
#            print(linenum,':level change to',level)


        if x.startswith("jerror_t JEventProcessor") :   # inside init or fini
            if x.find("::init(") >= 0 :
                in_finit = True
            if x.find("::fini(") >= 0 :
                in_finit = True

                
        if in_finit and dlevel < 0 and level == 0 :   # just left init or fini
            in_finit = False

            
        if x.find("RootFillLock(") >= 0 :
            if f_locked :
                printout.append("  Found RootFillLock at line "+str(linenum)+"but code was already FillLocked")
            if w_locked :
                printout.append("  Found RootFillLock at line "+str(linenum)+"but code was already WriteLocked")
            if in_finit :
                printout.append("  Found RootFillLock inside init or fini, locks are unnecessary there, see line "+str(linenum)) 
            f_locked = True
            locks_used = True
            lock_level = level
            level_unlocked = 0
#            printout.append('lock level',lock_level)
            
        if x.find("RootWriteLock(") >= 0 :
            if w_locked :
                printout.append("  Found RootWriteLock at line "+str(linenum)+"but code was already WriteLocked")
            if f_locked :
                printout.append("  Found RootWriteLock at line "+str(linenum)+"but code was already FillLocked")
            if in_finit :
                printout.append("  Found RootWriteLock inside init or fini, locks are unnecessary there, see line "+str(linenum))
            w_locked = True
            locks_used = True            
            lock_level = level
            level_unlocked = 0

            
        if x.find("RootFillUnLock(") >= 0 :

            if f_locked :
                if level == lock_level :   # only unlock if at the same block level.  
                    f_locked = False
                else :
                    level_unlocked = level
            else :
                printout.append("  Found RootFillUnLock at line "+str(linenum)+"but code was not FillLocked")

                
        if x.find("RootUnLock(") >= 0 :

            if w_locked : 
                if level == lock_level :   # only unlock if at the same block level.  
                    w_locked = False
                else :
                    level_unlocked = level
            else :
                printout.append("  Found RootUnLock at line "+str(linenum)+" but code was not WriteLocked")
                
    
    
        if not f_locked and not x.startswith("dTreeInterface") :   # DTreeInterface handles its own locks
    
            if (x.find("->Fill(") >= 0) or (x.find(".Fill(") >= 0) :
                fills_used = True
                if w_locked :
                    printout.append("  Histogram write locks are unnecessary, should be changed to fill locks, see line "+str(linenum))
                elif not in_finit :
                    printout.append("  ERROR: found fill while not root fill locked, check line "+str(linenum))
                    printout.append(line)
                        

        if x.find("continue") >= 0 or x.find("break") >= 0:
            cb_level = level
            cb_w_lockstate = w_locked
            cb_f_lockstate = f_locked
                    
                
        if f_locked or w_locked : 
        
            probably_ok = False
            thing = ""

            if level == 0 and not warned:
                printout.append("  Warning: still locked at line "+str(linenum))
                warned = True
                
            if x.find("continue") >= 0 :
                thing = "continue"
                if level > lock_level :
                    probably_ok = True
                if level == level_unlocked :                
                    probably_ok = True         # no warnings

            
            if x.find("break") >= 0 :
                thing = "break"
                if level > lock_level :
                    probably_ok = True
                if level == level_unlocked :                
                    probably_ok = True         # no warnings
    
                    
            if x.find("return") >= 0 :
                thing = "return"
                if level == level_unlocked :   # eg if (something) { unlock; return; }
                    probably_ok = True         # no warnings
     
                    
            if not probably_ok :
                if thing == "break" or thing == "return" :
                    printout.append("  WARNING: found "+thing+" inside a lock, check line "+str(linenum))

                
    f.close()

    if fills_used and not locks_used :
        printout.append("  Locks were not used in this file")

    
    return printout       


######################################################################################################################################################


import sys
import os

nargs = len(sys.argv)

if nargs<2 or sys.argv[0] == "-h" or sys.argv[0] == "--h" or sys.argv[0] == "--help":
    exit("This script scans GlueX/Hall D plugin code to check for root locks surrounding histogram fills.\nProvide either the filename or the name of the directory 2 levels above, eg 'monitoring'\nUsage: python checklocks.py filename\n")
    
w_locked=False
f_locked=False

fsname=sys.argv[1]

if not os.path.isdir(fsname) :

    if not os.path.exists(fsname):
        exit("Cannot find "+fsname)

    thisfilereport = check_this_file(fsname)

    if len(thisfilereport) == 0 :
        print("ok")
    else :
        for thing in thisfilereport:
            print(thing)
    
else :  # assume this is a directory like monitoring

    goodfiles = []
    badfiles = []
    report = []
    
    for plugindir in os.listdir(fsname) :
        #qprint(plugindir)
        fullpath = fsname+"/"+plugindir

        if os.path.isdir(fullpath) :
            for x in os.listdir(fullpath) :
                filename = fullpath + "/" + x

                if filename.endswith(".cc") and x.startswith("JEventProcessor") :
                    #print(filename)
                    thisreport = check_this_file(filename)

                    if len(thisreport) == 0 :
                        goodfiles.append(filename)
                    else :
                        badfiles.append(filename)

                        reportline = []
                        reportline.append(filename)
                        reportline.extend(thisreport)
                        report.append(reportline)



    if len(goodfiles) > 0 :
        print("These files look ok:")
        for thing in goodfiles:
            print(thing)
        print("")

    if len(badfiles) > 0 :        
        print("These files do not look ok:")
        for thing in badfiles:
            print(thing)
        print("")
        print("List of complaints:")

        for line in report:
            for thing in line:                
                print(thing)
            print("")


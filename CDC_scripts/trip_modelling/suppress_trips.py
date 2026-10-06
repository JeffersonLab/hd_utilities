# already ran get_all_eff to dump efficiency histo contents into eff_from_histo directory

import os
import subprocess
import glob
import rcdb
import shutil

testing = False  # just process one file
tolerance = 0.1
reporting_threshold = 0.85  # complain about unexpected efficiency below this


def main():

    dirs = ["eff_trip_suppressed", "eff_ts_combined"]
    for d in dirs:
        if not os.path.exists(d):
            os.makedirs(d)

    filedir = "eff_from_histo"
    filelist = os.listdir(filedir)
    filelist.sort()

    firstrun = True

    for file in filelist:

        run = file[4:10]

        with open(filedir + '/' + file,"r") as f:
            list = f.readlines()

        eff = []
        for x in list:
            eff.append(float(x))

        set_blanks(run, eff)

        newfile = get_filename(run)

        with open(get_filename(run), "w") as f:
            for x in eff:
                f.write(f"{x:.2f}")
                f.write("\n")
        


        if firstrun:
            start = run
            latest = run
            different = False
            firstrun = False
        else:
            previous = latest
            latest = run
            different = test_different(start, latest, tolerance)
            
        if different:
            startfile = get_filename(start)
            shutil.copy(startfile, get_combined_filename(start, previous))
            start = latest
                        
        if testing:  
            break

    # at end of loop output combo file for last group of runs
    if not different:
        startfile = get_filename(start)
        shutil.copy(startfile, get_combined_filename(start, latest))
        
    


def get_filename(run):
    return "eff_trip_suppressed/eff_" + run + ".txt"

def get_combined_filename(run1, run2):
    return "eff_ts_combined/eff_" + run1 + "_" + run2 + ".txt"

def add_to_bad_list(run):
    fbad = open("badfiles.txt", "a")
    fbad.write(run)
    fbad.write("\n")
    fbad.close()
    return  

def test_different(run1, run2, tolerance):
    fname1 = get_filename(run1)
    fname2 = get_filename(run2)

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

    return different


def set_blanks(run, eff) :

    r = int(run)

    B7 = [233, 234, 235, 236, 237, 238, 239, 240]
    B7.extend([300, 301, 302, 303, 304, 305, 306])
    B7.extend([374, 375, 376, 377, 378, 379, 380, 381, 382])

    H13 = [2437, 2438, 2439, 2440, 2441, 2442, 2443]
    H13.extend([2620, 2621, 2622, 2623, 2624, 2625, 2626, 2627])
    H13.extend([2809, 2810, 2811, 2812, 2813, 2814, 2815, 2816])

    H14 = [2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451]
    H14.extend([2628, 2629, 2630, 2631, 2632, 2633, 2634, 2635])
    H14.extend([2817, 2818, 2819, 2820, 2821, 2822, 2823, 2824])

    H15 = [2452, 2453, 2454, 2455, 2456, 2457, 2458, 2459]
    H15.extend([2636, 2637, 2638, 2639, 2640, 2641, 2642])
    H15.extend([2826, 2827, 2828, 2829, 2830, 2831, 2832, 2833])

    I14 = [3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010]
    I14.extend([3206, 3207, 3208, 3209, 3210, 3211, 3212, 3213])
    I14.extend([3415, 3416, 3417, 3418, 3419, 3420, 3421, 3422])

    I15 = [3011, 3012, 3013, 3014, 3015, 3016, 3017, 3018]
    I15.extend([3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221])
    I15.extend([3423, 3424, 3425, 3426, 3427, 3428, 3429, 3430])

    I16 = [2825, 3019, 3020, 3021, 3022, 3023, 3024, 3025]
    I16.extend([3222, 3223, 3224, 3225, 3226, 3227, 3228, 3229])
    I16.extend([3431, 3432, 3433, 3434, 3435, 3436, 3437, 3438])

    # low efficiency low gain board
    I24 = [3079, 3080, 3081, 3082, 3083, 3084, 3085]
    I24.extend([3286, 3287, 3288, 3289, 3290, 3291, 3292, 3293, 3294])
    I24.extend([3495, 3496, 3497, 3498, 3499, 3500, 3501, 3502])

    dead_straws = [244, 709, 2354, 2384, 3086, 3089, 3214, 3349]
    
    dead_before_recalib = [3003, 3005, 3006, 3212, 3415, 3419, 3420, 3422]

    low_gain_before_recalib = [2808, 3008, 3210, 3416] # 0.8

    lower_gain_before_recalib = [3211]  # 0.6



    
    # flaky pedestals


    straw10_95 = [150097, 150099, 150100, 150101, 150110, 150116, 150117,
                  150119, 150122, 150128, 150129, 150133, 150134, 150136, 150142,
                  150154, 150156, 150157, 150184, 150193, 150214, 150228, 150250,
                  150251, 150265, 150289, 150290, 150304, 150317, 150320, 150321,
                  150322, 150323, 150324, 150325, 150326, 150327, 150328, 150329,
                  150330, 150331, 150332, 150333, 150334, 150335, 150337, 150339,
                  150340, 150341, 150342, 150343,
                  150345, 150346, 150347, 150348, 150349, 150351, 150352, 150353,
                  150355, 150357, 150358, 150359, 150360, 150361, 150364, 150365,
                  150366, 150367, 150368, 150369, 150370, 150371, 150372, 150373,
                  150374, 150375, 150379, 150380, 150382, 150383, 150384, 150385,
                  150387, 150389, 150390, 150391, 150392, 150393, 150396, 150397,
                  150398, 150400, 150401, 150402, 150403, 150404, 150406, 150407,
                  150410, 150411, 150412, 150413, 150414, 150415, 150416, 150419,
                  150424, 150426, 150427, 150428, 150429, 150432, 150435, 150439,
                  150440, 150444, 150446, 150447, 150448, 150449, 150451, 150452,
                  150454, 150455, 150456, 150467, 150468, 150469, 150470, 150471,
                  150472, 150473, 150476, 150477, 150479, 150480, 150481, 150482,
                  150483, 150484, 150485, 150486, 150487, 150490, 150491, 150492,
                  150493, 150495, 150496, 150497, 150498, 150499, 150500, 150501,
                  150502, 150503, 150504, 150505, 150506, 150508, 150509, 150511,
                  150513, 150519, 150520, 150521, 150522, 150523, 150524, 150525,
                  150528]

    straw10_90 =  [150107, 150248, 150279, 150303, 150350, 150356, 150363,
                   150465]

    ###


    straw97_95 = [150335, 150350, 150351, 150352, 150353, 150355, 150356,
                  150357, 150360, 150361, 150368, 150369, 150374, 150375,
                  150379, 150380, 150382, 150383, 150384, 150385, 150387,
                  150389, 150390, 150391, 150392, 150393, 150396, 150402,
                  150407, 150410, 150411, 150412, 150413, 150414, 150415,
                  150416, 150419, 150424, 150426, 150427, 150428, 150429,
                  150432, 150447, 150448, 150449, 150451, 150452, 150454,
                  150455, 150456, 150465, 150467, 150468, 150469, 150470,
                  150471, 150472, 150473, 150476, 150477, 150479, 150480,
                  150481, 150482, 150483, 150484, 150485, 150486, 150487,
                  150490, 150491, 150492, 150493, 150495, 150499, 150500,
                  150501, 150502, 150503, 150504] 

    straw97_90 = [150333, 150334, 150337, 150339, 150340, 150348, 150349,
                  150363, 150364, 150365, 150366, 150367, 150370, 150371,
                  150372, 150373, 150400, 150401, 150403, 150404, 150406,
                  150435, 150439, 150440, 150444, 150446]

    straw97_85 = [150252, 150258, 150260, 150261, 150262, 150263, 150284,
                  150288, 150330, 150331, 150332, 150341, 150347]

    straw97_80 = [150254, 150255, 150256, 150257, 150264, 150266, 150267,
                  150268, 150275, 150276, 150280, 150281, 150291, 150293,
                  150295, 150310, 150314, 150329, 150342, 150343, 150345,
                  150346]

    straw97_75 = [150248, 150250, 150251, 150270, 150271, 150273, 150289,
                  150290, 150294, 150305, 150306, 150307, 150309, 150311,
                  150312, 150313, 150315, 150316, 150320, 150321, 150322,
                  150323, 150324, 150325, 150327, 150328]

    straw97_70 = [150265, 150279, 150303, 150317, 150326]

    straw97_65 = [150304]

    
    ###

    straw292_95 = [150102, 150107, 150110] 

    straw292_90 = [150097, 150099, 150100, 150101, 150121] 

    straw292_85 = [150116, 150117, 150119, 150120, 150122, 150123, 150125,
                   150126, 150127, 150128, 150130, 150137, 150138, 150139,
                   150140, 150141, 150143, 150144, 150145, 150146, 150147,
                   150148, 150150, 150151, 150152, 150153, 150158, 150159,
                   150165, 150168] 

    straw292_80 = [150129, 150131, 150132, 150133, 150134, 150136, 150142,
                   150154, 150156, 150157, 150160, 150162, 150163, 150164,
                   150169, 150170, 150171, 150173, 150174, 150175, 150176,
                   150181, 150182, 150183] 

    straw292_75 = [150184] 
    


    ###
    

    straw2130_95 =  [150107, 150114, 150116, 150117, 150119, 150120, 150121,
                     150122, 150123, 150125, 150126, 150130, 150133, 150134] 

    straw2130_85 =  [150131, 150132] 

    straw2130_0 =  [150137, 150138, 150139, 150140, 150141, 150142, 150143,
                    150144, 150145, 150146, 150147, 150148, 150150, 150151,
                    150152, 150153, 150154, 150156, 150157, 150158, 150159,
                    150160, 150162, 150163, 150164, 150165, 150168, 150169,
                    150170, 150171, 150173, 150174, 150175, 150176, 150181,
                    150182, 150183, 150184] 
    
    ###
    
    straw2460_95 =  [150152, 150153, 150154, 150158, 150159, 150160, 150163,
                     150164, 150165, 150168, 150169, 150170, 150171, 150173,
                     150175, 150181, 150182] 

    straw2460_90 =  [150150, 150151] 

    straw2460_85 =  [150147, 150148] 

    straw2460_80 =  [150143, 150144, 150145, 150146] 

    straw2460_75 =  [150130, 150133, 150138, 150139, 150140, 150141, 150142] 

    straw2460_70 =  [150116, 150119, 150120, 150122, 150123, 150125, 150126,
                     150127, 150128, 150129, 150131, 150132, 150134, 150136,
                     150137] 

    straw2460_65 =  [150107, 150110, 150114, 150117, 150121] 

    ###
    
    straw3066_95 =  [150374, 150380, 150383, 150389, 150390, 150392, 150397,
                     150520]

    straw3066_90 =  [150393, 150412, 150424]

    straw3066_85 =  [150419]

    straw3066_80 =  [150413]

    straw3066_70 =  [150414, 150415, 150416]

    ###

    straw3067_95 =  [150393, 150419, 150424]

    straw3067_90 =  [150413, 150414, 150415, 150416]

    ###

    straw3068_95 =  [150374, 150380, 150383, 150385, 150387, 150389, 150390,
                     150391, 150392, 150396, 150397, 150398, 150513]

    straw3068_90 =  [150520]

    straw3068_85 =  [150412]

    straw3068_80 =  [150424]

    straw3068_70 =  [150393]

    straw3068_65 =  [150419]

    straw3068_0 =  [150414, 150415, 150416]

    ###

    straw3070_95 =  [150393, 150412, 150424]

    straw3070_90 =  [150419]

    straw3070_85 =  [150413]

    straw3070_80 =  [150414, 150415, 150416]

    ###
    
    straw3271_95 =  [150372, 150390, 150392, 150412]

    straw3271_90 =  [150393, 150419, 150424]

    straw3271_80 =  [150413, 150414, 150416]

    straw3271_75 =  [150415]

    ###

    straw3481_95 =  [150229, 150234, 150237, 150327, 150390, 150392, 150393, 150398, 150412, 150424, 150427, 150451, 150520]

    straw3481_90 =  [150413, 150419]

    straw3481_85 =  [150414, 150415, 150416]

    ###

    straw3483_95 =  [150393, 150412, 150419, 150424, 150520]

    straw3483_90 =  [150413]

    straw3483_85 =  [150414, 150415, 150416]
    
    ###
    '''
    150103-150156 - CDC B7 tripped on July 14
    150135-150145 CDC H13, I14 tripped on July 17 close to the end of 150134
    150137 CDC Entire G ring tripped & restored about 10 minutes later.
    150137-150145 CDC H15, I16 tripped
    150137-150145 - CDC H14, I15 unstable or off
    150147 - CDC H13, I14 tripped & restored
    150173-150174 - CDC B7 tripped & restored slowly
    150174-150176 - CDC H13, I14 tripped & restored
    150201-150212 - CDC B7 tripped on July 22
    150216 - CDC B7 tripped on July 22 at 21:15, 7 mins before the end of 150215, was restored midway through 150216
    150245-150293 - CDC B7 tripped on July 24 at the start of 150245, restored before 150294
    150335- -150346 - CDC B7 tripped on July 31
    '''

    blank_B7 = False

    if (r >= 150103 and r <= 150156) or (r == 150174) or (r >= 150201 and r <= 150212):
        blank_B7 = True

    if (r == 150216):
        blank_B7 = True
        
    # ignoring 150333-150334
    if (r >= 150245 and r <= 150293) or r == 150328:
        blank_B7 = True

    if (r == 150334) or (r == 150337):
        blank_B7 = True

    if (r >= 150339 and r <= 150346):
        blank_B7 = True
        
        
    blank_H13 = False
    blank_I14 = False
    if (r >= 150135 and r <= 150145) or (r == 150147) or (r >= 150174 and r <= 150176):
        blank_H13 = True
        blank_I14 = True

        
    blank_H14 = False
    blank_I15 = False
    if  r >= 150137 and r <= 150145:
        blank_H14 = True
        blank_I15 = True
               

    blank_H15 = False
    blank_I16 = False
    if r >= 150137 and r <= 150145:
        blank_H15 = True
        blank_I16 = True

        
    for i in range(0,3522):
        straw = i+1

        eff_histo = eff[i]

        eff[i] = 1

        if straw in dead_straws:
            eff[i] = 0
            
        if r < 150188: # baseline recalib
            if straw in dead_before_recalib:
                eff[i] = 0
            if straw in low_gain_before_recalib:
                eff[i] = 0.8
            if straw in lower_gain_before_recalib:
                eff[i] = 0.6

            
        if blank_B7 and straw in B7:
            eff[i] = 0


        if blank_H13 and straw in H13:
            eff[i] = 0
                
        if blank_H14 and straw in H14:
            eff[i] = 0
                
        if blank_H15 and straw in H15:
            eff[i] = 0
                
        if blank_I14 and straw in I14:
            eff[i] = 0
                
        if blank_I15 and straw in I15:
            eff[i] = 0

        if blank_I16 and straw in I16:
            eff[i] = 0

                
        if straw in I24:
            eff[i] = 0.95



            
        if straw == 10:  # wandering pedestal
            if r in straw10_95:
                eff[i] = 0.95
            elif r in straw10_90:
                eff[i] = 0.9
                        
        if straw == 97:  # wandering pedestal
            if r in straw97_95:
                eff[i] = 0.95
            elif r in straw97_90:
                eff[i] = 0.9
            elif r in straw97_85:
                eff[i] = 0.85
            elif r in straw97_80:
                eff[i] = 0.8
            elif r in straw97_75:
                eff[i] = 0.75
            elif r in straw97_70:
                eff[i] = 0.7
            elif r in straw97_65:
                eff[i] = 0.65

        if straw == 292: # wandering pedestal
            if r in straw292_95:
                eff[i] = 0.95
            elif r in straw292_90:
                eff[i] = 0.9
            elif r in straw292_85:
                eff[i] = 0.85
            elif r in straw292_80:
                eff[i] = 0.9
            elif r in straw292_75:
                eff[i] = .75
                
        if straw == 2130: # wandering pedestal
            if r in straw2130_95:
                eff[i] = 0.95
            elif r in straw2130_85:
                eff[i] = 0.85
            elif r in straw2130_0:
                eff[i] = 0
            
        if straw == 2460: # wandering pedestal
            if r in straw2460_95:
                eff[i] = 0.95
            elif r in straw2460_90:
                eff[i] = 0.9
            elif r in straw2460_85:
                eff[i] = 0.85
            elif r in straw2460_80:
                eff[i] = 0.8
            elif r in straw2460_75:
                eff[i] = 0.75
            elif r in straw2460_70:
                eff[i] = 0.7
            elif r in straw2460_65:
                eff[i] = 0.65

                



# also need to look at 3065

                
        if straw == 3066:
            if r in straw3066_95:
                eff[i] = 0.95
            elif r in straw3066_90:
                eff[i] = 0.9
            elif r in straw3066_85:
                eff[i] = 0.85
            elif r in straw3066_80:
                eff[i] = 0.8
            elif r in straw3066_70:
                eff[i] = 0.7

        if straw == 3067:
            if r in straw3067_95:
                eff[i] = 0.95
            elif r in straw3067_90:
                eff[i] = 0.9
                
        if straw == 3068:
            if r in straw3068_95:
                eff[i] = 0.95
            elif r in straw3068_90:
                eff[i] = 0.9
            elif r in straw3068_85:
                eff[i] = 0.85
            elif r in straw3068_80:
                eff[i] = 0.8
            elif r in straw3068_70:
                eff[i] = 0.7
            elif r in straw3068_65:
                eff[i] = 0.65
            elif r in straw3068_0:
                eff[i] = 0

        if straw == 3070:
            if r in straw3070_95:
                eff[i] = 0.95
            elif r in straw3070_90:
                eff[i] = 0.9
            elif r in straw3070_85:
                eff[i] = 0.85
            elif r in straw3070_80:
                eff[i] = 0.8

        if straw == 3271:
            if r in straw3271_95:
                eff[i] = 0.95
            elif r in straw3271_90:
                eff[i] = 0.9
            elif r in straw3271_80:
                eff[i] = 0.8
            elif r in straw3271_75:
                eff[i] = 0.75

        if straw == 3481:
            if r in straw3481_95:
                eff[i] = 0.95
            elif r in straw3481_90:
                eff[i] = 0.9
            elif r in straw3481_85:
                eff[i] = 0.85
                
        if straw == 3483:
            if r in straw3483_95:
                eff[i] = 0.95
            elif r in straw3483_90:
                eff[i] = 0.9
            elif r in straw3483_85:
                eff[i] = 0.85
                
                

                
        # print out remaining problems.  Ignore cases where straws are shadowed by dead boards
        if eff[i] == 1 and eff_histo < reporting_threshold :
            known = False
            
            if blank_B7 and straw>=455 and straw<=463:
                known = True
                
            if blank_H13 and straw>=2093 and straw<=2096:
                known = True
            if blank_H13 and straw>=2262 and straw<=2267:
                known = True
            if blank_H13 and straw>=3011 and straw<=3013:
                known = True
            if blank_H13 and (straw == 3215 or straw <= 3423):
                known = True

            if blank_H15 and (straw >= 2107 and straw <= 2111):
                known = True
            if blank_H15 and (straw >= 2276 and straw <= 2281):
                known = True
            if blank_H15 and (straw >= 3026 and straw <= 3030):
                known = True
            if blank_H15 and (straw >= 3230 and straw <= 3233):
                known = True
            if blank_H15 and (straw >= 3439 and straw <= 3442):
                known = True

            if blank_I14 and straw>=2806 and straw<=2808:
                known = True

            if straw >= 3066 and straw <= 3068 and (r==150393 or (r>=150413 and r<=150416)):
                known = True

            if r>=150413 and r<=150416:
                if straw == 3481 or straw == 3483:
                    known = True
                    
            if (r==150173 or r==150330 or r==150332 or r==150335) and straw in B7:
                known = True

                    
            if straw == 3011:  # in front of dead straw.
                known = True
                
            if not known:
                print(run, eff_histo, straw)

    return



if __name__ == "__main__":
    main()

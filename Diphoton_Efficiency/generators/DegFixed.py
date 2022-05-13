## azizahm, JLAB, 2021
## Will generate a textfile of desired theta, phi, momentum for 2 photons
## Usage : python DegFixed.py

## check file location and output if necessary
import os
from random import seed
from random import random
import math

## create a local text file 
script_dir = os.getcwd()
file_name = "/diphoton.txt" # "/diphoton_single.txt"
file_location  = script_dir + file_name

## ## Defining all the values that we will be using ## ## 
run_number = 100000
event_number = 10000000 # number of events, first event is 'event 0'
number_of_particles = 2 # 1 for _single

# particle_number is a range that goes from 1-number_of_particles
# and will be the loop number
particleID = 1
photon_mass = 0
photon_charge = 0

def fn_diff(P1, P2, Theta1, Theta2, Phi, Phi2): 

#from a spherical vector to cartesian components

  Px1 = P1*math.sin(Theta1)*math.cos(Phi)
  Py1 = P1*math.sin(Theta1)*math.sin(Phi)
  Pz1 = P1*math.cos(Theta1) 

  Px2 = P2*math.sin(Theta2)*math.cos(Phi2)
  Py2 = P2*math.sin(Theta2)*math.sin(Phi2)
  Pz2 = P2*math.cos(Theta2)
  
  return Px1, Py1, Pz1, Px2, Py2, Pz2



# %time ## to check time 
with open(file_location, 'w') as file1:
  for j in range(event_number):
    #if j%50000 ==0: print(j) # to check time 
    seed()
    file1.write("%s " % run_number) #notice the space that has been included afterwards it
    file1.write( "%s " % j) # the event number
    file1.write("%s\n" % number_of_particles)
    for i in range(1, number_of_particles+1):
      file1.write("%s " % i) # particle number, starts at one
      file1.write("%s " % particleID)
      file1.write("%s\n" % photon_mass)

      file1.write("   %s " % photon_charge)
      if i==1:
        ## for every event to be unique we randomise the following
        theta1 = math.radians(random() * 1.1 + 2) #MCW is in radians
        theta2 = math.radians(random() * 1.1 + 2) #MCW is in radians
        phi = math.radians(random() * 20) # Some small range in theta and phi, so we can look at various distances between the two showers
        phi2 = math.radians(random() * 20)
        p1 = 3 #GeV
        p2 = 2 # GeV
        
        px1, py1, pz1, px2, py2, pz2 = fn_diff(p1, p2, theta1, theta2, phi, phi2)
        file1.write("%s " % px1)
        file1.write("%s " % py1)
        file1.write("%s " % pz1)
        file1.write("%s\n" % p1)

      else: # comment out this block for _single
        file1.write("%s " % px2)
        file1.write("%s " % py2)
        file1.write("%s " % pz2)
        file1.write("%s\n" % p2)

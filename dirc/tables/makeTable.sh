#!/bin/csh

# CCDB commands to create base directories
ccdb mkdir DIRC
ccdb mkdir DIRC/LUT
ccdb mkdir DIRC/North
ccdb mkdir DIRC/South

# Add LUT TTree as resource to CCDB: 
## Copy lut_all_flat.root to /group/halld/www/halldweb/html/resources/DIRC/LUT/
## Set JANA_CALIB_CONTEXT to correct location (private ccdb.sqlite3 or MySQL)
## Execute command line to add resource, which will create CCDB commands and ask for your confirmation
jresource -a DIRC/LUT/lut_all_flat.root 0- https://halldweb.jlab.org/resources

# CCDB: Create map table which connects resource files to run numbers
ccdb mktbl DIRC/LUT/lut_map -r 1 map_name=string
ccdb add DIRC/LUT/lut_map -r 60000- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/lut_file_name.txt # (format for path in text file is in resource dir eg. /DIRC/LUT/lut.root)

# CCDB: Create tables for timing offsets, etc.
ccdb mktbl DIRC/base_time_offset -r 1 t0_North t0_South
ccdb mktbl DIRC/North/timing_offsets -r 6912 t0
ccdb mktbl DIRC/South/timing_offsets -r 6912 t0
ccdb mktbl DIRC/North/channel_status -r 6912 status=int
ccdb mktbl DIRC/South/channel_status -r 6912 status=int

# CCDB: Add time offset tables (loop over relevant run numbers
ccdb add DIRC/base_time_offset -r 0- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/base_time_offset.txt
ccdb add DIRC/North/timing_offsets -r 0- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/dummy.txt
ccdb add DIRC/South/timing_offsets -r 0- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/dummy.txt
ccdb add DIRC/North/channel_status -r 0- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/dummy.txt
ccdb add DIRC/South/channel_status -r 0- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/dummy.txt

ccdb add Magnets/Solenoid/solenoid_map -r 60000- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/solenoid_map.txt
ccdb add PHOTON_BEAM/endpoint_energy -r 60000- /work/halld2/home/jrsteven/2018-dirc/builds/hd_utilities/dirc/tables/endpoint_energy.txt


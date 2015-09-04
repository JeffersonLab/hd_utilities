#!/bin/bash

function show_help
{
    cat <<EOF
Description:

Run b1pi_test.sh for a particular configuration of the GlueX software. The environment can be set in advance or a version.xml file can be supplied. In the latter case the file will be used to set-up the GlueX environment.

Usage:

  my_b1pi_test.sh [-n <number of events>] [-t <number of threads>] \\
    [-v <vertex string>] [-d <b1pi_test script directory>] \\
    [-v <version.xml file>]

Example:

  mkdir my_test_dir # make a test directory
  cd my_test_dir # cd into it
  export GLUEX_TOP /home/username/gluex_top # define location of the top of the 
                                            # GlueX software tree
  export B1PI_TEST_DIR /home/username/b1pi_test # define the location of the
                                                # b1pi_test files
  b1pi_test.sh -n 10000 -f /home/username/my_version.xml # do the test for 10 k
                                                         # events using versions
                                                         # in my_version.xml
EOF
}

while getopts "h?v:f:n:t:d:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    f)  output_file=$OPTARG
        ;;
    n)  NEVENTS=$OPTARG
	;;
    v)  VERTEX=$OPTARG
	;;
    t)  NTHREADS=$OPTARG
	;;
    d)  B1PI_TEST_DIR=$OPTARG
    esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

if [ ! -d "$B1PI_TEST_DIR" ]
    then
    echo "error: location of scripts and macros not found"
    echo "       location tried = \"$B1PI_TEST_DIR\""
    exit 1
fi

if [ -z "$NEVENTS" ]
    then
    echo "info: number of events not defined, using default value $nevents"
    NEVENTS=$nevents
fi

if [ -z "$NTHREADS" ]
    then
    echo "info: number of threads not defined, using default value $nthreads"
    NTHREADS=$nthreads
fi

if [ -z "$VERTEX" ]
    then
    echo "info: vertex parameters not defined, using default values $vertex"
    VERTEX=$vertex
fi

echo NEVENTS = $NEVENTS
echo VERTEX = $VERTEX
echo NTHREADS = $NTHREADS
echo B1PI_TEST_DIR = $B1PI_TEST_DIR

echo "Copying script files and macros ..."
cp -pv $B1PI_TEST_DIR/* .
cp -pv $B1PI_TEST_DIR/macros/* .

echo "Running genr8 ..."
genr8 -r1501 -M${NEVENTS} -Ab1_pi.ascii < b1_pi.input

echo "Converting generated events to HDDM ..."
genr8_2_hddm -V"${VERTEX}" b1_pi.ascii 

echo "Creating control.in file ..."
cat - << EOF > control.in

INFILE 'b1_pi.hddm'
TRIG ${NEVENTS}
OUTFILE 'hdgeant.hddm'
RNDM 123
HADR 1

EOF

echo "Running hdgeant ..."
hdgeant

echo "Running mcsmear ..."
mcsmear -PJANA:BATCH_MODE=1 hdgeant.hddm

echo "Running hd_root with danarest ..."
hd_root -PJANA:BATCH_MODE=1 --nthreads=$NTHREADS -PPLUGINS=danarest hdgeant_smeared.hddm

echo "Running hd_root with b1pi_hists & monitoring_hists ..."
hd_root -PJANA:BATCH_MODE=1 --nthreads=$NTHREADS -PPLUGINS=b1pi_hists,monitoring_hists dana_rest.hddm

echo "Create plots"
root -b -q mk_pics.C

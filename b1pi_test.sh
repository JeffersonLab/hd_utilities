#!/bin/bash

show_help ()
{
    echo <<EOF
Usage:
  b1pi_test.sh [-n <number of events>] [-t <number of threads>] -v <vertex string> [-d <b1pi_test script directory>]
Example:
  b1pi_test.sh -n 1000 -t 4 -v "0 0 50 80"
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

echo "Leftovers: $@"
echo NEVENTS = $NEVENTS
echo VERTEX = $VERTEX
echo NTHREADS = $NTHREADS
echo B1PI_TEST_DIR = $B1PI_TEST_DIR

if [ -z "$NEVENTS" ]
    then
    echo "error: number of events not defined"
    exit 1
fi

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
hd_root -PJANA:BATCH_MODE=1 --nthreads=Ncores -PPLUGINS=danarest hdgeant_smeared.hddm

echo "Running hd_root with b1pi_hists & monitoring_hists ..."
hd_root -PJANA:BATCH_MODE=1 --nthreads=Ncores -PPLUGINS=b1pi_hists,monitoring_hists dana_rest.hddm

echo "Create plots"
root -b -q mk_pics.C

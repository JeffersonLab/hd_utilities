#!/bin/bash

run_period=$1
run_number=$2
batch=$3
version=ver$3
threadnb=$4

first_run_number=$5

dividenb=$(echo "256 / $threadnb" | bc)
halld_version_set="halld_recon-5.9.0"
sed 's,BATCH,'$batch',g' script_nersc_test.temp > script_nersc_test.sh
#sed 's,VERSION,'$version',g;s,THREADNB,'$threadnb',g' jana_recon_nersc.temp > jana_recon_nersc.config
sed 's,THREADNB,'$threadnb',g' jana_recon_nersc.temp > jana_recon_nersc.config
#sed 's,DIVIDENB,'$dividenb',g' script_nersc_multi_test.temp > script_nersc_multi_test.py
#sed 's,DIVIDENB,'$dividenb',g' script_nersc_multi_test_temp.py > script_nersc_multi_test.py
chmod +x script_nersc_test.sh
chmod +x script_nersc_multi_test.py
#scp -r ../launch-$batch perlmutter-p1.nersc.gov:/global/cfs/cdirs/m3120/
nersc_launch_dir=/global/cfs/cdirs/m3120/launch-$batch
#nersc_launch_dir=$PWD
workflow=recon_${run_period}_${version}_batchNERSC-multi
swif2 create $workflow -site nersc/perlmutter --maxconcurrent 100
swif2 run $workflow
idir=/mss/halld/RunPeriod-$run_period/rawdata/Run$run_number
files_full_path=($idir/*.evio)
files_basename=($(basename -a $idir/*.evio))
##files_basename=()
# Populate files_basename array
##for f in "${files_full_path[@]}"; do
##    files_basename+=("$(basename "$f")")
##done
# Create a new combined array
files=()
for i in "${!files_full_path[@]}"; do
    files+=("-input ${files_basename[i]} mss:${files_full_path[i]}")
done
# Print the new array for verification
##for entry in "${files[@]}"; do
##    echo "$entry"
##done
number_of_evios=${#files_full_path[@]}
echo "Run period: $run_period - run number: $run_number - number of evio files: $number_of_evios - divided by: $dividenb"
#number_of_nodes=$(echo "$number_of_evios / $dividenb" | bc)
number_of_nodes=$(echo "($number_of_evios + $dividenb - 1) / $dividenb" | bc)
echo "Number of nodes asked: $number_of_nodes" 
i=0
name=GLUEX_recon_$run_number
command1="swif2 add-job -workflow $workflow -name $name "
while [ $i -le $number_of_nodes ]; do
    ##echo "i = $i"
    start=$((i * $dividenb))   # Calculate the starting index
    end=$((start + $dividenb)) # Calculate the ending index
    # Print 5 values from the array
    ##echo "i = $i -> Values: ${files_full_path[@]:start:5}"
    ##node_mss=${files_full_path[@]:start:5}
    node_mss=${files[@]:start:${dividenb}}
    #node_cache="${node_mss/'/mss'/'mss:/mss'}"
    node_cache=$node_mss
    RUNDIR=$(printf "RUN%06d/FILE%03d" "$run_number" "$i")
    node_volatile=/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-$run_period/recon/$version/$RUNDIR
    echo "mkdir -p $node_volatile"
    mkdir -p $node_volatile
    #####command2+=" -name $name -input $node_cache -output match:$RUNDIR/* $node_volatile"
    command2+=" $node_cache -output match:$RUNDIR/* $node_volatile"
    #echo "swif2 add-job -workflow $workflow -name $name -input $node_cache -output match:RUN$run_number/FILE$i/* $node_volatile"
    ((i++))  # Increment i
done
command3=" -sbatch -A m3120 --volume=\"$nersc_launch_dir:/launch-$batch\" --image=docker:jeffersonlab/gluex_almalinux_9:latest --module=cvmfs --time=5:00:00 -N $number_of_nodes --tasks-per-node=1 --cpus-per-task=256 --qos=regular -C cpu :: $nersc_launch_dir/script_nersc_multi_test.sh $nersc_launch_dir /launch-$batch/script_nersc_test.sh /launch-$batch/jana_recon_nersc.config halld_recon/$halld_version_set $dividenb"
#command3=" -sbatch -A m3120 --volume=\"$nersc_launch_dir:/launch-$batch\" --image=docker:jeffersonlab/gluex_almalinux_9:latest --module=cvmfs --time=3:00:00 -N $number_of_nodes --tasks-per-node=1 --cpus-per-task=256 --qos=regular -C cpu :: $nersc_launch_dir/script_nersc_multi_test.sh $nersc_launch_dir $nersc_launch_dir/script_nersc_test.sh $nersc_launch_dir/jana_recon_nersc.config halld_recon/halld_recon-5.1.0"
command4=$command1$command2$command3
echo $command4 > exec_$run_number.sh
#echo "$nersc_launch_dir/script_nersc_multi.sh $nersc_launch_dir $nersc_launch_dir/script_nerscc.sh $nersc_launch_dir/jana_recon_nersc.config halld_recon/halld_recon-5.0.0" > exec.sh
chmod +x exec_$run_number.sh
##scp ../launch-$batch perlmutter-p1.nersc.gov:$nersc_launch_dir/
if [ $run_number == "$first_run_number" ]; then
    echo "in if"
    scp -r ../launch-$batch perlmutter-p1.nersc.gov:/global/cfs/cdirs/m3120/
fi
./exec_$run_number.sh

# run all jobs
#CMD = ['srun', '-n', SLURM_JOB_NUM_NODES, LAUNCHDIR+'/run_shifter_multi.sh']
#CMD += [workdir]          # arg 1:  top-level directory for job
#CMD += [SCRIPTFILE]       # arg 2:  script to run inside shifter (all subsequent args are eventually passed to this script)
#CMD += [CONFIG]           # arg 3:  JANA config file
#CMD += [RECONVERSION]     # arg 4:  sim-recon version
#srun -n $SLURM_JOB_NUM_NODES /path/to/launchdir/run_shifter_multi.sh test script.sh c.cfg halld0404

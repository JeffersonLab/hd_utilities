#!/bin/bash

rp=$1
ve=$2
pe=$3

volatile_recon_dir=/volatile/halld/offsite_prod/RunPeriod-$rp/recon/$pe/$ve/REST

volatile_hists=/volatile/halld/offsite_prod/RunPeriod-$rp/recon/$pe/$ve/hists

volatile_flux_dir=/volatile/halld/offsite_prod/RunPeriod-$rp/recon/$pe/$ve/tree_PSFlux
##mkdir -p $volatile_hists

ls -1 $volatile_recon_dir > rp-$rp-files-to-copy-to-cache.txt

mss_dir=/mss/halld/RunPeriod-$rp/rawdata

mss_recon_dir=/mss/halld/RunPeriod-$rp/recon/$ve

cache_recon_dir=/cache/halld/RunPeriod-$rp/recon/$ve

work_dir=/work/halld/home/gxproj4/offsite_prod/RunPeriod-$rp

volatile_dir=/volatile/halld/recon/RunPeriod-$rp/$ve

rnb_tab=( $( cat rp-$rp-files-to-copy-to-cache.txt ) )

rm good-list-$rp.txt
touch good-list-$rp.txt
rm bad-list-$rp.txt
touch bad-list-$rp.txt

i1=0
i2=0
i3=0
i4=0
i5=0
i6=0
i7=0
for rnb in "${rnb_tab[@]}"; do
    echo "rnb: $rnb"
    file_tab=($(ls $mss_dir/Run$rnb/hd_rawdata_??????_???.evio))
    i7=0
    ##mkdir -p $volatile_hists/$rnb
    for file in "${file_tab[@]}"; do
	read rnb < <(echo "$file" | grep -o '[0-9]\+' | tail -n 2)
        read enb < <(echo "$file" | grep -o '[0-9]\+' | tail -n 1)
	#echo "rnb: $rnb - enb: $enb"
	#echo "file: $file"
	file1a=$cache_recon_dir/REST/$rnb/dana_rest_${rnb}_${enb}.hddm
	file2a=$cache_recon_dir/tree_PSFlux/$rnb/tree_PSFlux_${rnb}_${enb}.root
	file3a=$cache_recon_dir/hists/${rnb}/hd_root_${rnb}_${enb}.root
	file1b=$mss_recon_dir/REST/$rnb/dana_rest_${rnb}_${enb}.hddm
	file2b=$mss_recon_dir/tree_PSFlux/$rnb/tree_PSFlux_${rnb}_${enb}.root
	file3b=$mss_recon_dir/hists/${rnb}/hd_root_${rnb}_${enb}.root
	#filehe=$work_dir/job_info_${rnb}_$enb/helicity.log
	filear=$volatile_dir/REST/$rnb/dana_rest_${rnb}_${enb}.hddm
	fileaf=$volatile_dir/tree_PSFlux/$rnb/tree_PSFlux_${rnb}_${enb}.root
	fileah=$volatile_dir/hists/${rnb}/hd_root_${rnb}_${enb}.root
	if { [[ -f $file1a ]] || [[ -f $file1b ]]; } && \
	   { [[ -f $file2a ]] || [[ -f $file2b ]]; } && \
	   { [[ -f $file3a ]] || [[ -f $file3b ]]; }; then
	    #echo "$rnb $enb" >> good-list.txt
	    ((i1++))
	else
	    echo "$rnb $enb" >> bad-list-$rp.txt
	    #echo "Missing outputs $rnb $enb"
	    ((i2++))
	    ((i7++))
	fi
	if [[ -e $file1a || -e $file1b ]]; then
	    #if [ -f "$filear" ]; then
	    #cp $filear $volatile_recon_dir/$rnb/
	    #fi
	    ((i3++))
	fi
	if [[ -e $file2a || -e $file2b ]]; then
	    #if [ -f "$filear" ]; then
	    #cp $fileaf $volatile_flux_dir/$rnb/
	    #fi
	    ((i4++))
	fi
	if [[ -e $file3a || -e $file3b ]]; then
	    #if [ -f "$filehe" ]; then
	    #cp $filehe $volatile_hists/$rnb/helicity_${rnb}_$enb.log
	    #fi
	    #if [ -f "$fileah" ]; then
	    #cp $fileah $volatile_hists/$rnb/
	    #fi
	    ((i5++))
	fi
	((i6++))
    done
    if (( i7 == 0 )); then
	echo "$rnb $enb" >> good-list-$rp.txt
    fi
done
echo "Total number of correct files with REST, PSFlux, and hists: $i1"
echo "Total number of incorrect files: $i2"
echo "Total number of correct files with REST: $i3"
echo "Total number of correct files with PSFlux: $i4"
echo "Total number of correct files with hists: $i5"
echo "Total number of files: $i6"

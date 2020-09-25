ver=$1
runnb=(30496)
wname=wo-dark-matter-version-$ver
path_out=/work/halld/home/ijaegle/code_testing/bsm/dark-matter/full-mc-$ver
gencfgdi=wo_cfg-$ver
mkdir -p $path_out
mkdir -p $gencfgdi
evtnb=10000
path_in=/work/halld/home/ijaegle/code_testing/bsm/dark-matter/root
h=0
for process in ae_to_ape; do
    for ffile in `ls $path_in/${process}_*.root`; do
	fname=$(basename $ffile)
	fname=${fname/.root/""}
	genfig=$gencfgdi/whizard_$fname.cfg
	sed 's,MYPROCESS,'${process}',g; s,LHEFILE,'${ffile}',g' config_whizard.temp > $genfig
	for gve in 3 4; do
	    for bkgname in None; do
		bkg=$bkgname
		if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
		    bkg=Random
		fi
		for rnb in "${runnb[@]}"; do
		    for gto in whizard; do
			name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_${gto}_$fname
			outdir=$path_out/$name
			mkdir -p $outdir
			config=$gencfgdi/MC_${name}.config
			sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g; s,FILENAME,'${genfig}',g' MC_wo_v$ver.temp > $config
			echo "gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2 mc nb $h"
			gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2
			h=$(($h+1))
		    done
		done
	    done
	done
    done
done
echo 'h ' $h

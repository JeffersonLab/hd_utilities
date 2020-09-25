##runnb=(61323)
runnb=(30496 61323 61585)
wname=slurm-run-gen_pi0-test0-with-core1_iter1
path=/work/halld/home/ijaegle/$wname
mkdir -p $path
config_dir=cfg-$wname
mkdir -p $config_dir
evtnb=10000

for gve in 3 4; do
    for bkgname in None; do
 	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    for gto in pi0; do
		name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_$gto
		outdir=$path/$name
		mkdir -p $outdir
		config=$config_dir/MC_$name.config
		echo $config
		sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_pi0_dev0.temp > $config
		echo "./gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2"
		gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2
		#gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 
	    done
	done
    done
done

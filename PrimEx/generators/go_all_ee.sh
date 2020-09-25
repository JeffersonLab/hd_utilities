#runnb=(30496)
runnb=(61323)
naming=gen_ee
version=production-0
farm=slurm
wname=${naming}-${farm}-$version
cfgdir=cfgdir-$wname
wname=${naming}-${farm}-$version
path=/work/halld/home/ijaegle/$wname
mkdir -p $path
mkdir -p $cfgdir
evtnb=1000000
for gve in 3 4; do
    #for bkgname in None Random:recon-2017_01-ver03; do
    for bkgname in None; do
	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    for gto in ee; do
		for reaction in 2 3; do
		    name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_r${reaction}_$gto
		    outdir=$path/$name
		    mkdir -p $outdir
		    genfig=$cfgdir/gen_ee_$name.cfg
		    sed 's,REACTION,'${reaction}',g' config_ee.temp > $genfig
		    config=$cfgdir/MC_$name.config
		    sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g; s,FILENAME,'${genfig}',g' MC_dev.temp > $config
		    echo "./gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2"
		    if [ "${farm}" == "slurm" ]; then
			gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2
		    fi
		    if [ "${farm}" == "inter" ]; then
			gluex_MC.py $config $rnb $evtnb cleanmcsmear=0
		    fi
		done
	    done
	done
    done
done

runnb=(61706)
naming=primex_pi0_he4
version=production-pi0
farm=inter
wname=${naming}-${farm}-$version
cfgdir=cfg-dir-$wname
wname=${naming}-${farm}-$version
path=/work/halld/home/ijaegle/$wname
mkdir -p $path
mkdir -p $cfgdir
evtnb=100
for gve in 4; do
#    for bkgname in None BeamPhotons Random:recon-2017_01-ver03; do
    for bkgname in None; do
	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    for gto in primex_eta_he4; do
		#for gto in ee; do
		name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_$gto
		outdir=$path/$name
		mkdir -p $outdir
		config=$cfgdir/MC_$name.config
		sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_dev.temp > $config
		echo "./gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2"
		if [ "${farm}" == "slurm" ]; then
		    gluex_MC.py $config $rnb $evtnb cleangeant=0 cleanmcsmear=0 batch=2
		fi
		if [ "${farm}" == "inter" ]; then
		    gluex_MC.py $config $rnb $evtnb cleangeant=0 cleanmcsmear=0
		fi
	    done
	done
    done
done

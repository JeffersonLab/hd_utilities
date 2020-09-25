runnb=(30496)
#runnb=(30496 61323)
#runnb=(61323 30496)
#naming=compton-v0-default-all-tagged
naming=gen_ee
#naming=compton-v2-with-ccal-and-no-B-field
#version=gluex-with-ccal
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
#    for bkgname in None BeamPhotons Random:recon-2017_01-ver03; do
    for bkgname in None Random:recon-2017_01-ver03; do
	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    #for gto in compton_simple omega_3pi; do
	    for gto in ee; do
		#for gto in ee; do
		name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_$gto
		outdir=$path/$name
		mkdir -p $outdir
		config=$cfgdir/MC_$name.config
		sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_dev.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_v0b.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_v0.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_v1.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_v2.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_CCAL.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_ori.temp > $config
		#sed 's,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,GTO,'${gto}',g; s,WNAME,'${wname}',g' MC_var.temp > $config
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

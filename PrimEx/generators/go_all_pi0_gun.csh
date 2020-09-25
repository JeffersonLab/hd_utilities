##runnb=(61323)
runnb=(30480)
wname=slurm-pi0-particle_gun_AT_FCAL
path=/work/halld/home/ijaegle/$wname
mkdir -p $path
config_dir=$path/cfg-$wname
mkdir -p $config_dir
evtnb=100000
max=8
for gve in 3 4; do
    for bkgname in None; do
 	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    for gto in gamma pi0 eta; do
		for theta in 1 2 3 4 5 6 7 8 9 10; do
		    for tkin in 0.1 0.3 0.6 0.9 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.0 6.5 7.0 7.5 8.0 8.5 9.0 9.5 10.0; do
			h=0
			while [ $h -lt $max ]; do
			    name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_${gto}_${theta}deg_${energy}GeV
			    outdir=$path/$name
			    mkdir -p $outdir
			    config=$config_dir/MC_$name.config
			    echo $config
			    sed 's,VERNB,'${h}',g; s,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,WNAME,'${wname}',g' MC_pi0_gun_dev0.temp > $config
			    echo "./gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2"
			    gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2
			    #gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 
			    h=$(($h+1))
			done
		    done
		done
	    done
	done
    done
done

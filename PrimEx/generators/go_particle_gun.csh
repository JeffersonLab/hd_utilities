#(eg. 101=gamma, 103=electron, 107=pi0, 108=pi+, 109=pi-, 114=proton, 117=eta). 
#   particle  momentum  theta  phi  delta_momentum delta_theta delta_phi
#KINE PID      MOMENTUM  THETA  0.0  1.0            2.0         360.0
#config_particle_gun.temp
#MC_pi0_gun_dev0.temp

runnb=(30480)
wname=slurm-particle_gun_AT_FCAL_with_geant_output_mono_particle_no_thres
path=/work/halld/home/ijaegle/$wname
wrap=/work/halld/home/ijaegle/gluex_MCwrapper
mkdir -p $path
config_dir=$path/cfg-$wname
mkdir -p $config_dir
evtnb=10000
for gve in 3; do
    for bkgname in None; do
 	bkg=$bkgname
	if [ "${bkgname}" == "Random:recon-2017_01-ver03" ]; then
	    bkg=Random
	fi
	for rnb in "${runnb[@]}"; do
	    #for pid in 101 107 117; do
	    for pid in 101; do
		for theta in 1 2 3 4 5 6 7 8 9 10; do
		    for tkin in 0.01 0.02 0.03 0.04 0.05 0.06 0.07 0.08 0.09 0.11 0.12 0.13 0.14 0.15 0.16 0.17 0.18 0.19 2.2 2.4 2.6 2.8 0.1 0.5 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 0.2 0.3 0.4 0.6 0.7 0.8 0.9 1.2 1.4 1.6 1.8; do
			#for tkin in 0.1 0.5 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 1.2 1.4 1.6 1.8 0.2 0.3 0.4 0.6 0.7 0.8 0.9; do
			name=Geant${gve}_bkg_${bkg}_runnb_${rnb}_gen_${pid}_${theta}deg_${tkin}GeV
			outdir=$path/$name
			mkdir -p $outdir
			gunfig=$wrap/config_particle_gun_${pid}_${theta}deg_${tkin}GeV.cfg
			config=$config_dir/MC_$name.config
			echo $config
			sed 's,TKIN,'${tkin}',g; s,THETA,'${theta}',g; s,PID,'${pid}',g' $wrap/config_particle_gun_mono.temp > $gunfig
			#sed 's,TKIN,'${tkin}',g; s,THETA,'${theta}',g; s,PID,'${pid}',g' $wrap/config_particle_gun.temp > $gunfig
			sed 's,TKIN,'${tkin}',g; s,THETA,'${theta}',g; s,PID,'${pid}',g; s,GVE,'${gve}',g; s,BKGNAME,'${bkgname}',g; s,RNB,'${rnb}',g; s,OUTDIR,'${outdir}',g; s,WNAME,'${wname}',g' MC_gun.temp > $config
			echo "./gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 batch=2"
			gluex_MC.py $config $rnb $evtnb cleangeant=0 cleanmcsmear=0 batch=2
			#gluex_MC.py $config $rnb $evtnb cleanmcsmear=0 
		    done
		done
	    done
	done
    done
done

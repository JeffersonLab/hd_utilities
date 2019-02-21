#!/bin/csh
#Usage: 


set i = 0

set basedir = $1
cd basedir

foreach dir (`find $basedir/*.* -maxdepth 0 -type d`)
	
	# set fsizesearch = `find $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root -size +10k`
	# set fsizesearch = `find $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root -size +10M`
	set fsizesearch = `find $dir/hd_root.root -size +100k`
	
	if($fsizesearch == "" ) then
		echo "no file found in dif" $dir 
		# rm $dir/log/log_REDO.log
		rm $dir/log_REDO.log
		setenv JANA_CALIB_CONTEXT "variation=mc_sim1"
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/?????_*/*smeared.hddm -o $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/?????_*/*smeared.hddm -o $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root >& $dir/log/log_REDO.log &
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/hddm/*smeared.hddm -o $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root >& $dir/log/log_REDO.log &
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/*smeared.hddm -o $dir/hd_root_particle_gun_photon_gun_E_scan_030300_000.root >& $dir/log_REDO.log &
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/*smeared.hddm -o $dir/hd_root_particle_gun_photon_gun_E_scan_030300_000.root >& $dir/log_REDO.log &
		qsub $dir/ReplaceAndRun.csh
	else 
		echo "file found"
	endif
	
	
	
	# if( -e $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root) then
		# # echo "file found"
	# else
		# echo "Resubmitting in directory: " $dir
		
		# # hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/?????_*/*smeared.hddm -o $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root
		# rm $dir/log/log_REDO.log
		# hd_root -PPLUGINS=photon_gun_hists -PNTHREADS=4  $dir/?????_*/*smeared.hddm -o $dir/root/hd_root_particle_gun_photon_gun_E_scan_030300_000.root >& $dir/log/log_REDO.log &
	# endif
	# hd_root -PPLUGINS=photon_gun_hists  $dir/hddm/*smeared.hddm -o photon_gun_$i.root >& log_$i.log &
	# hd_root -PPLUGINS=photon_gun_hists  $dir/hddm/*smeared.hddm -o photon_gun_$i.root -PNTHREADS=8
	@ i++
end

echo "Done"

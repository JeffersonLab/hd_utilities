now=$(date)
echo $now
squeue -u jlab
count=$(squeue -u jlab | wc -l)
echo "Number of jlab job in the queue: $count"
count=$(squeue | wc -l)
echo "Number of job in the queue: $count"
count=$(squeue | grep "  R" | wc -l)
echo "Number of job running: $count"
evio_nb=$1
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/dana*_000.hddm
co_dana1=$(ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/dana*_000.hddm | wc -l)
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/std.out
co_std1=$(ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/std.out | wc -l)
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE*/core*
chmod og+rw running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE*/core*
echo 'Multiple jobs per node'
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/*000/dana*
co_dana2=$(ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/*000/dana* | wc -l)
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/*/std*_000.out
co_std2=$(ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE$evio_nb/*/std*_000.out | wc -l)
ls -lrth running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE*/*/core*
chmod og+rw running_dir2/jobs/gxproj4/*_recon_*/*/RUN*/FILE*/*/core*
source check_space.sh
source make_list.sh
file="list-queue.txt"
echo "Nb of dana 1 : $co_dana1, nb of std.out: $co_std1"
echo "Nb of dana 2 : $co_dana2, nb of std.out: $co_std2"
sum=$(awk '{sum += $(NF-1)} END {print sum}' "$file")
echo "Number of nodes requested in total: $sum"
file="list-jlab-queue.txt"
sum=$(awk '{sum += $(NF-1)} END {print sum}' "$file")
echo "Number of nodes requested by jlab: $sum"
now=$(date)
echo $now

cd /pscratch/sd/j/jlab/swif
echo 'Amount of data on pscratch'
du -sh
echo 'Amount of data on cfs'
cd /global/cfs/cdirs/m3120
du -sh
cd
echo "jlab quota"
myquota -u jlab
#echo "ijaegle quota"
#myquota -u ijaegle
cd

rp=$1
ve=$2
nb=$3
ba=$4
swif2 status recon_${rp}_${ve}_batchNERSC-multi
#ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN09*/FILE000/RUN09*/FILE000/dana*
echo "Number of run processed"
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE$nb/RUN*/FILE$nb/exitcode_0.txt | wc -l
echo "Number of run not moved yet"
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE$nb/RUN*/FILE$nb/dana*_000.hddm
echo "Number of rawdata file coming back"
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE$nb/RUN*/FILE$nb/hd_rawdata_??????_???.evio
rm /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN??????/FILE???/RUN??????/FILE???/hd_rawdata_??????_???.evio
swif2 status recon_${rp}_${ve}_batchNERSC-multi
echo "Number of runs cooked:" 
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE$nb/RUN*/FILE$nb/dana*_000.hddm | wc -l
echo "Number of runs cooked w/ tgz:" 
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE$nb/RUN*/FILE$nb/*_000.tgz | wc -l
echo "Number of files cooked w/ and w/o tgz:" 
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/RUN*/FILE*/RUN*/FILE*/dana*.hddm | wc -l
echo "Number of runs cooked & cached:"
ls /cache/halld/RunPeriod-$rp/recon/$ba/REST/*/*_000.hddm | wc -l
echo "Number of runs cooked & mssed:"
ls /mss/halld/RunPeriod-$rp/recon/$ba/REST/*/*_000.hddm | wc -l
echo "Number of runs checked & ordered not moved yet:"
ls -lrth /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/$ba/REST/*/dana*_000.hddm
echo "Number of runs check & ordered"
ls /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/$ba/REST/ | wc -l
echo "Number of REST files checked & ordered:"
ls /volatile/halld/offsite_prod/RunPeriod-$rp/recon/$ve/$ba/REST/*/dana*_???.hddm | wc -l

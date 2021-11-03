#!/bin/bash
hd_interface=/home/marki/git/hd_interface
hdi_conversion=/home/marki/git/hd_utilities/hdi_conversion
rm -rf halld_recon
git clone git@github.com:jeffersonlab/halld_recon
#cp -pr halld_recon.save halld_recon # instead of clone for testing to save time
cd halld_recon
echo git filter-repo \\ > ../command.sh
awk '{print "    --path "$1" \\"}' < $hdi_conversion/filter.txt >> ../command.sh
echo >> ../command.sh
. ../command.sh
git branch| awk '{print "git branch -D "$1}' | bash | grep -v "Deleted branch"
git mv src old_src
git commit -m "src moved to old_src" | grep -v "rename"
find $hd_interface -type f -name \*.cc | $hdi_conversion/move_cc.sh
find $hd_interface -type f -name \*.h | $hdi_conversion/move_h.sh
exit
git commit -m "move done" | grep -v "rename"
git rm -r old_src | grep -v "rm "
git commit -m "old_src deleted" | grep -v "delete mode"
git status
